#include <pthread.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <ctime>
#include "URLParser.h"
#include "HTTPLib.h"
#include "csapp.h"
#include "Spider.h"

using namespace std;

// Prototype declarations for the worker and parser threads.
void* worker_thread(void*);
void* parser_thread(void*);

// This class represents a lockable object. It is intended
// to be used as base class for other domain specific classes
// to extend.
class LockObj {
public:
    LockObj() {
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);        
    }

    ~LockObj() {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    void lock() {
        pthread_mutex_lock(&mutex);
    }

    void unlock() {
        pthread_mutex_unlock(&mutex);
    }

    void wait() {
        pthread_cond_wait(&cond, &mutex);
    }

    void signal() {
        pthread_cond_signal(&cond);
    }

    void broadcast() {
        pthread_cond_broadcast(&cond);
    }
    
private:
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
};

// Represents a unit of work for worker threads:
class Work: public LockObj {
public:
    Work(url_t url, int depth, string doc) {
        this->url   = url;
        this->depth = depth;
        this->doc   = doc;
    }

    Work(url_t url, int depth) {
        this->url   = url;
        this->depth = depth;
    }

    url_t& getURL() {
        return url;
    }

    string FullURL() {
        return "http://" + url.host + url.file;
    }

    int getDepth() {
        return depth;
    }

    string& getDoc() {
        return doc;
    }

private:
    url_t  url;       // The URL to fetch.
    int    depth;     // The current depth of this unit of work.
    string doc;       // The string document that was fetched.
};

// This class represents the data (context) that is passed to
// a thread using pthread_create.
class ThreadCtx {
public:
    ThreadCtx(int id, int maxdepth) {
        this->id       = id;
        this->maxdepth = maxdepth;
    }

    int MaxDepth() {
        return maxdepth;
    }

    int Id() {
        return id;
    }
    
private:
    int maxdepth;  // The maximum depth to crawl.
    int id;        // The identifier of the thread.
};

// This is the work queue that holds all the work to
// be done by the worker threads.  It is a queue of
// Work objects.
class WorkQueue: public LockObj {
public:
    WorkQueue() {
    }

    ~WorkQueue() {
    }

    int size() {
        return work.size();
    }

    bool IsEmpty() {
        return work.size() == 0;
    }
    
    Work* take() {
        Work* w = work.front();
        work.erase(work.begin());
        return w;
    }

    void add(Work* w) {
        work.push_back(w);
    }

private:
    vector<Work*>   work;   // A vector of Work objects.
};

// Represents a simple atomic counter object. It provides
// exclusive access to a thread that invokes it's methods.
class Counter: public LockObj {
public:
    Counter(int start) {
        this->start = start;
    }

    int val() {
        return start;
    }
    
    void inc() {
        lock();
        start++;
        unlock();
    }

    void dec() {
        lock();
        start--;
        unlock();
    }

    bool Zero() {
        return start == 0;
    }

    bool NotZero() {
        return start != 0;
    }
private:
    int start;  // A basic counter variable.
};

// A table of document objects that are retrieved by
// the worker threads.
class DocTbl: public LockObj {
public:
    DocTbl()  { }
    ~DocTbl() { }
    bool contains(string url) {
        map<string,string>::iterator it = docs.find(url);
        return it != docs.end();
    }
    void add(string url, string doc) {
        lock();
        docs[url] = doc;
        unlock();
    }
    int size() {
        return docs.size();
    }
    void dump() {
        map<string,string>::iterator it = docs.begin();
        while (it != docs.end()) {
            cout << it->first << endl;
            it++;
        }
    }
private:
    // A map from host+file names to the documents
    // retrieved from http://<host>/<file>.
    map<string,string> docs;
};

class IndexSet: public LockObj {
public:
    IndexSet() : fcnt(0) { }
    ~IndexSet() { }

    bool contains(string url) {
        map<string,string>::iterator it = idx.find(url);
        return it != idx.end();        
    }

    void add(string url, string doc) {
        lock();
        // Create the file path/name for the output file.
        time_t t = time(0);  // get current time
        
        stringstream file;
        file << "web/idxfiles/" << t << ".txt";
        string fname = file.str();
        ofstream docf(fname.c_str(), ios::app);
        docf << doc;
        docf.close();

        // Add the entry to the indexer file list.
        ofstream idxfile("web/index.txt", ios::app);
        idxfile << fname << " # " << url << endl;
        idxfile.close();
        
        // Add the url -> file mapping.
        idx[url] = file.str();
        unlock();
    }

    int size() {
        return idx.size();
    }
    
private:
    map<string,string> idx;
    int fcnt;
};

// Here we declare the actual worker and parser queues:
WorkQueue workerQ;
WorkQueue parserQ;

// This represents the number of Work objects that are
// currently in flight. This is used to determine when
// we no longer have any more work to be done (in addition
// to an empty work queue).
Counter in_flight(0);

// Here is where we declare the document table:
IndexSet docs;

// A typedef to make iterator declarations shorter:
typedef vector<url_t>::const_iterator iter;

// This function is here to demonstrate how we would
// crawl in a sequential fasion. It is here primarly
// as reference to better understand the threaded code.
void* sequential(void* argv) {
    ThreadCtx* ctx = (ThreadCtx*)argv;
    
    while (workerQ.size() != 0) {
        Work* w = workerQ.take();

        if (w->getDepth() > ctx->MaxDepth())
            continue;
        
        if (http_fetch(w->getURL(), w->getDoc()) == 0) {
            vector<url_t> urls = parse_urls(w->getDoc());

            for (iter i = urls.begin(); i != urls.end(); i++) {
                url_t  url  = *i;
                cout << "http://" << url.host << url.file << endl;
                Work*  work = new Work(url, w->getDepth()+1);
                workerQ.add(work);
            }
        }

        delete w;
    }

    return NULL;    
}

// This function is called by the worker_thread to do
// the work of fetching the resource specified by the
// URL in the Work object.  We pass the thread context
// to aid in debugging output.
void worker_do_work(Work* w, ThreadCtx* ctx) {
    if (w->getDepth() <= ctx->MaxDepth() &&
        http_fetch(w->getURL(), w->getDoc()) == 0) {
        parserQ.lock();
        cout << "[worker " << ctx->Id() << "] sending " << w->FullURL() << endl;
        parserQ.add(w);        
        parserQ.signal();
        parserQ.unlock();
        w->wait();
        docs.add(w->FullURL(), w->getDoc());
    }
    else {
        in_flight.dec();
        workerQ.signal();
    }
}

// This is the worker thread. It is invoked indirectly
// through the call to pthread_create.
void* worker_thread(void* argv) {
    ThreadCtx* ctx = (ThreadCtx*) argv;
    workerQ.lock();
    while (in_flight.NotZero() || !workerQ.IsEmpty()) {
        if (workerQ.IsEmpty()) {
            if (in_flight.NotZero()) {
                workerQ.signal();
                workerQ.wait();
                continue;
            }
            else {
                break;
            }
        }
        
        Work* w = workerQ.take();
        in_flight.inc();
        workerQ.signal();
        workerQ.unlock();
        
        if (!docs.contains(w->FullURL())) {
            // do work:
            worker_do_work(w, ctx);
        }
        else {
            in_flight.dec();
        }
        delete w;
        workerQ.lock();
    }

    workerQ.broadcast();
    workerQ.unlock();
    delete ctx;
    return NULL;
}

// This is a function that represents the main work
// that is accomplished by the parser thread.  It
// is called by the parser_thread function.
void parser_do_work(Work* w, ThreadCtx* ctx) {
    vector<Work*> nws;

    vector<url_t> urls = parse_urls(w->getDoc());
    for (iter i = urls.begin(); i != urls.end(); i++) {
        url_t  url = *i;
        Work*  nw  = new Work(url, w->getDepth()+1);
        nws.push_back(nw);
    }

    workerQ.lock();
    for (vector<Work*>::const_iterator j = nws.begin(); j != nws.end(); j++)
        workerQ.add(*j);
    
    in_flight.dec();
    workerQ.signal();
    workerQ.unlock();
}

// A boolean value that is used to communicate to the
// parser thread that it has more work to do.
bool PARSER_MORE_WORK = true;

// The parser thread that is indirectly called by the
// the call to pthread_create.
void* parser_thread(void* argv) {
    ThreadCtx* ctx = (ThreadCtx*) argv;
    parserQ.lock();
    while (PARSER_MORE_WORK) {
        if (parserQ.IsEmpty()) {
            parserQ.wait();
            continue;
        }
        Work* w = parserQ.take();
        cout << "[parser] received " << w->FullURL() << endl;
        
        parserQ.unlock();
        parser_do_work(w, ctx);
        w->signal();
        parserQ.lock();
    }
    parserQ.unlock();
    return NULL;
}

// Crawl is the main public routine for creating all the
// threads and reaping the threads when they are complete.
void crawl(std::string& root_url, int n, int d) {
    // Create the first item of work:
    Work* work  = new Work(parse_url_str(root_url), 0);
    workerQ.add(work);

    // Create the parser thread:
    pthread_t parser_tid;
    ThreadCtx* pctx = new ThreadCtx(n, d);
    Pthread_create(&parser_tid, NULL, parser_thread, pctx);

    // Create the worker threads:
    pthread_t worker_tids[n];
    for (int i = 0; i < n; i++) {
        ThreadCtx* ctx = new ThreadCtx(i, d);
        Pthread_create(&worker_tids[i], NULL, worker_thread, ctx);
    }

    // Join each of the worker threads:
    for (int i = 0; i < n; i++) {
        Pthread_join(worker_tids[i], NULL);
    }

    // Tell the parser that no work work is available:
    PARSER_MORE_WORK = false;
    parserQ.signal();

    // Join the parser thread with the main thread:
    Pthread_join(parser_tid, NULL);

    cout << "processed " << docs.size() << " urls." << endl;
}
