/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the 
 *     GET method to serve static and dynamic content.
 */
#include <string.h>
#include "csapp.h"
#include "tiny.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void serve_search(int fd, char *filename, char *cgiargs);
void serve_spider(int fd, char *filename, char *cgiargs);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, 
                 char *shortmsg, char *longmsg);

char * replace(char const * const original,
               char const * const pattern,
               char const * const replacement);

int main(int argc, char **argv) 
{
    int listenfd, connfd, port;
    char *indexfile=NULL;
    struct sockaddr_in clientaddr;
    unsigned int clientlen = sizeof(clientaddr);

    /* Check command line args */
//1-modified by chengyang 
    if (argc != 3) {
        fprintf(stderr, "usage: %s <port> <index file>\n", argv[0]);
        exit(1);
    }
    port = atoi(argv[1]);
/*add by chengyyang*/
    indexfile = argv[2]; 
    build_index(indexfile); 

    printf("tiny webserver listing on port %d\n", port);
    listenfd = Open_listenfd(port);
    while (1) {
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        doit(connfd);
        Close(connfd);
    }
}
/* $end tinymain */

/*
 * doit - handle one HTTP request/response transaction
 */
/* $begin doit */
void doit(int fd) 
{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;
  
    /* Read request line and headers */
    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);
    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET")) { 
        clienterror(fd, method, "501", "Not Implemented",
                    "Tiny does not implement this method");
        return;
    }
    read_requesthdrs(&rio);

    /* Parse URI from GET request */
    //动态的存cgiargs[]中，静态页面存uri[]   
    is_static = parse_uri(uri, filename, cgiargs);

    if (is_static == 1) { /* Serve static content */
        // Is it a regular file with owner read permissions?
        if (stat(filename, &sbuf) < 0) {
            clienterror(fd, filename, "404", "Not found",
                        "Tiny couldn't find this file");
            return;
        }
        
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden",
                        "Tiny couldn't read the file");
            return;
        }
        // Serve the static file:
        serve_static(fd, filename, sbuf.st_size);
    }


    else if(is_static == 2){ /*server search*/
         // Is it a regular file with owner read permissions?
             serve_search(fd,filename,cgiargs);
    }

    else if(is_static == 3){ /*server search*/
         // Is it a regular file with owner read permissions?
             serve_spider(fd,filename,cgiargs);
    }


    else { /* Serve dynamic content */
        if (stat(filename, &sbuf) < 0) {
            clienterror(fd, filename, "404", "Not found",
                        "Tiny couldn't find this file");
            return;
        }
        
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden",
                        "Tiny couldn't read the file");
            return;
        }
        // Serve it dynamically:
        serve_dynamic(fd, filename, cgiargs);
    }
}
/* $end doit */

/*
 * read_requesthdrs - read and parse HTTP request headers
 */
/* $begin read_requesthdrs */
void read_requesthdrs(rio_t *rp) 
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    while(strcmp(buf, "\r\n")) 
        Rio_readlineb(rp, buf, MAXLINE);
    return;
}
/* $end read_requesthdrs */

/*
 * parse_uri - parse URI into filename and CGI args
 *             return 0 if dynamic content, 1 if static, 2 if search
 */
/* $begin parse_uri */
int parse_uri(char *uri, char *filename, char *cgiargs) 
{
    char *ptr;
    if (strstr(uri, "cgi-bin")) {  /* Dynamic content */
        ptr = index(uri, '?');
       if (ptr) {
            strcpy(cgiargs, ptr+1);
            *ptr = '\0';
        }
        else 
            strcpy(cgiargs, "");
        strcpy(filename, "./web/");
        strcat(filename, uri);
        return 0;
    }
    // Identifies search application
    else if (strstr(uri, "search")) {
        ptr = index(uri, '?');
        if (ptr) {
            strcpy(cgiargs, ptr+1);
            *ptr = '\0';
        }
        else 
            strcpy(cgiargs, "");
        strcpy(filename, "./web/");
        strcat(filename, uri);
        return 2;
    }    
 else if (strstr(uri, "spider")) {
        ptr = index(uri, '?');
        if (ptr) {
            strcpy(cgiargs, ptr+1);
            *ptr = '\0';
        }
        else 
        strcpy(cgiargs, "");
        strcpy(filename, "./web/");
        strcat(filename, uri);
        return 3;
    }  
    else {  /* Static content */
        strcpy(cgiargs, "");
        strcpy(filename, "./web/");
        strcat(filename, uri);
        if (uri[strlen(uri)-1] == '/')
            strcat(filename, "home.html");
        return 1;
    }

}
/* $end parse_uri */

/*
 * serve_static - copy a file back to the client 
 */
/* $begin serve_static */
void serve_static(int fd, char *filename, int filesize) 
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];
 
    /* Send response headers to client */
    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf));

    /* Send response body to client */
    srcfd = Open(filename, O_RDONLY, 0);
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Close(srcfd);
    Rio_writen(fd, srcp, filesize);
    Munmap(srcp, filesize);
}

//cheng 
void serve_search(int fd, char *filename, char *cgiargs) {
    char * ptr = NULL;
    char buf[MAXBUF*20];    //accommodate URLs
    char hdr[MAXBUF];       //store html header
    char * res=NULL;
printf("cgiargs:%s\n",cgiargs);

    if(ptr = strstr(cgiargs,"="))
    {
        if(strstr(ptr,"%20"))
        {
             res = replace(ptr+1,"%20"," "); // ascii替换为空格
        }
        else
        {
            res = replace(ptr+1, "+"," ");
        }
    }

printf("res:%s\n",res);
    sprintf(buf,"<html><head><title>Search</title></head><body>\n");
    
    url_list* urls_searched = search_index(res);

    int urlnums=0;
    url_list* head = urls_searched;
    for(;head != NULL;head = head->next)
        ++urlnums;
     
    sprintf(buf,"%s%d results for '%s':\n", buf,urlnums, res);
    sprintf(buf,"%s<ul>\n",buf);      
    if(urlnums)
    {
        url_list* list_head = urls_searched;
        while(urlnums--)
        {
            sprintf(buf,"%s<li><a href=\"%s\">%s</a></li>\n",buf,list_head->url,list_head->url);      
            list_head = list_head->next;
        }
    }
    sprintf(buf,"%s</ul><a href=\"/toogle.html\">Back to search form</a></body></html>",buf);
 
    /* Send response headers to client */
    sprintf(hdr, "HTTP/1.0 200 OK\r\n");
    sprintf(hdr, "%sServer: Tiny Web Server\r\n", hdr);
    sprintf(hdr, "%sContent-length: %zu\r\n", hdr, strlen(buf));
    sprintf(hdr, "%sContent-type: %s\r\n\r\n", hdr, "text/html");
    Rio_writen(fd, hdr, strlen(hdr));

    Rio_writen(fd, buf, strlen(buf));

    free(res);
    url_list* del = urls_searched;
    for(;urls_searched!=NULL; )
    {
        free(del);
        urls_searched = urls_searched->next; 
        del = urls_searched;
    }
}

/*
 * get_filetype - derive file type from file name
 */
void get_filetype(char *filename, char *filetype) 
{
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
}  
/* $end serve_static */

/*
 * serve_dynamic - run a CGI program on behalf of the client
 */
/* $begin serve_dynamic */
void serve_dynamic(int fd, char *filename, char *cgiargs) 
{
    char buf[MAXLINE], *emptylist[] = { NULL };

    /* Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));
  
    if (Fork() == 0) { /* child */
        /* Real server would set all CGI vars here */
        setenv("QUERY_STRING", cgiargs, 1); 
        Dup2(fd, STDOUT_FILENO);         /* Redirect stdout to client */
        Execve(filename, emptylist, environ); /* Run CGI program */
    }
    Wait(NULL); /* Parent waits for and reaps child */
}
/* $end serve_dynamic */

/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum, 
                 char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int) strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}
/* $end clienterror */

char * replace(char const * const original,
               char const * const pattern,
               char const * const replacement) {
    size_t const replen = strlen(replacement);
    size_t const patlen = strlen(pattern);
    size_t const orilen = strlen(original);

    size_t patcnt = 0;
    const char * oriptr;
    const char * patloc;

    // find how many times the pattern occurs in the original string
    for (oriptr = original; (patloc = strstr(oriptr, pattern)); (oriptr = patloc + patlen))
        {
            patcnt++;
        }

    {
        // allocate memory for the new string
        size_t const retlen = orilen + patcnt * (replen - patlen);
        char * const returned = (char *) malloc( sizeof(char) * (retlen + 1) );

        if (returned != NULL)
            {
                // copy the original string,
                // replacing all the instances of the pattern
                char * retptr = returned;
                for (oriptr = original; (patloc = strstr(oriptr, pattern)); oriptr = patloc + patlen)
                    {
                        size_t const skplen = patloc - oriptr;
                        // copy the section until the occurence of the pattern
                        strncpy(retptr, oriptr, skplen);
                        retptr += skplen;
                        // copy the replacement
                        strncpy(retptr, replacement, replen);
                        retptr += replen;
                    }
                // copy the rest of the string.
                strcpy(retptr, oriptr);
            }
        return returned;
    }
}




//cheng 
void serve_spider(int fd, char *filename, char *cgiargs) {
    char * ptr = NULL;
    char buf[MAXBUF*20];    //accommodate URLs
    char hdr[MAXBUF];       //store html header
    char* res  = NULL ;
    if(ptr = strstr(cgiargs,"="))
    {
        if(strstr(ptr,"%3A"))
        {
             res = replace(ptr+1,"%3A",":");
        }
        if(strstr(ptr,"%2F"))
        {
             res = replace(ptr+1,"%2F","/");
        }
       printf("res:%s LINE:%d\n",res,__LINE__);  //www.***/**/.com
printf("ptr:%s LINE:%d\n",ptr,__LINE__);  //www.***/**/.com
    }
printf("res:%s LINE:%d\n",res,__LINE__);  //www.***/**/.com

    //if(sscanf(cgiargs, "addr=%s&thread=%c&depth=%c",&addr,&t,&d)==3){
    //sscanf("addr=www.bvai.com","addr=%s\n",&addr);
   // printf("%d: addr=%s\n",__LINE__,addr);
    
    char addr[127];
    int thread;
    int depth;
    memset(addr,0,sizeof(addr));
    //memset(thread,0,sizeof(thread));
    //memset(depth,0,sizeof(depth));
    
    char* tmp = strtok(res,"&");
    sscanf(tmp,"%s",&addr);
    printf("addr=%s\n",addr);
     
    
    tmp = strtok(NULL,"&");
    sscanf(tmp,"thread=%d",&thread);
    printf("thread=%d\n",thread);
    
    tmp = strtok(NULL,"&");
    sscanf(tmp,"depth=%d",&depth);
    printf("depth=%d\n",depth);

    crawl_c(addr,thread,depth);
 
    
    sprintf(buf,"<html><head><title>Search</title></head><body>\n");
    sprintf(buf,"%s<h1 href='toogle.html'>already,enjoy search~</h1>\n",buf);
    sprintf(buf,"%s<ul>\n",buf);
    sprintf(buf,"%s<li> <a href=\"/spidy.html\">back to spidy~</a></li>\n",buf);
    sprintf(buf,"%s<li> <a href=\"/home.html\">back to home</a></li></body></html>\n",buf);
    
    // Send response headers to client 
    sprintf(hdr, "HTTP/1.0 200 OK\r\n");
    sprintf(hdr, "%sServer: Tiny Web Server\r\n", hdr);
    sprintf(hdr, "%sContent-length: %zu\r\n", hdr, strlen(buf));
    sprintf(hdr, "%sContent-type: %s\r\n\r\n", hdr, "text/html");
    Rio_writen(fd, hdr, strlen(hdr));

    Rio_writen(fd, buf, strlen(buf));

    //free(addr);
    //addr = NULL;
} 
    
/*
    
    url_list* urls_searched = search_index(res);

    int urlnums=0;
    url_list* head = urls_searched;
    for(;head != NULL;head = head->next)
        ++urlnums;
     
    sprintf(buf,"%s%d results for '%s':\n", buf,urlnums, res);
    sprintf(buf,"%s<ul>\n",buf);      
    if(urlnums)
    {
        url_list* list_head = urls_searched;
        while(urlnums--)
        {
            sprintf(buf,"%s<li><a href=\"%s\">%s</a></li>\n",buf,list_head->url,list_head->url);      
            list_head = list_head->next;
        }
    }
    sprintf(buf,"%s</ul><a href=\"/toogle.html\">Back to search form</a></body></html>",buf);
 
    // Send response headers to client 
    sprintf(hdr, "HTTP/1.0 200 OK\r\n");
    sprintf(hdr, "%sServer: Tiny Web Server\r\n", hdr);
    sprintf(hdr, "%sContent-length: %zu\r\n", hdr, strlen(buf));
    sprintf(hdr, "%sContent-type: %s\r\n\r\n", hdr, "text/html");
    Rio_writen(fd, hdr, strlen(hdr));

    Rio_writen(fd, buf, strlen(buf));

    free(res);
    url_list* del = urls_searched;
    for(;urls_searched!=NULL; )
    {
        free(del);
        urls_searched = urls_searched->next; 
        del = urls_searched;
    }
}
*/
