#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <termios.h>

#define LOG_TAG "RIL"
#include <utils/Log.h>
#include <cutils/properties.h>

int quectel_CM(int argc, char *argv[]);

#if 1 //quectel PLATFORM_VERSION >= "4.2.2"
#ifndef LOGD
#define LOGD ALOGD
#endif
#ifndef LOGE
#define LOGE ALOGE
#endif
#ifndef LOGI
#define LOGI ALOGI
#endif
#endif        

#define MAX_PATH 256
int ql_get_ndisname(char **pp_usbnet_adapter) {
    struct dirent* ent = NULL;  
    struct dirent* subent = NULL;  
    DIR *pDir, *pSubDir;  
    char dir[MAX_PATH], subdir[MAX_PATH];
    int fd;
    int find_usb_device = 0;
    int find_qmichannel = 0;

    *pp_usbnet_adapter = NULL;
    
    strcpy(dir, "/sys/bus/usb/devices");
    if ((pDir = opendir(dir)) == NULL)  {  
        LOGE("Cannot open directory: %s", dir);  
        return -ENODEV;  
    }  

    while ((ent = readdir(pDir)) != NULL) {
        char idVendor[5] = "";
        char idProduct[5] = "";
                  
        sprintf(subdir, "%s/%s/idVendor", dir, ent->d_name);
        fd = open(subdir, O_RDONLY);
        if (fd > 0) {
            read(fd, idVendor, 4);
            close(fd);
        //dbg_time("idVendor = %s\n", idVendor);
            if (strncasecmp(idVendor, "05c6", 4)) {
                continue;
            }
        } else {
            continue;
        }

        sprintf(subdir, "%s/%s/idProduct", dir, ent->d_name);
        fd = open(subdir, O_RDONLY);
        if (fd > 0) {
            read(fd, idProduct, 4);
            close(fd);
            //dbg_time("idProduct = %s\n", idProduct);
            if (strncasecmp(idProduct, "9003", 4) && strncasecmp(idProduct, "9215", 4)) {
                continue;
            }
        } else {
            continue;
        }
    
        LOGE("Find idVendor=%s, idProduct=%s", idVendor, idProduct);
        find_usb_device = 1;
        break;
    }
    closedir(pDir);

    if (!find_usb_device) {
        LOGE("Cannot find Quectel UC20/EC20");
        return -ENODEV;  
    }      

    sprintf(subdir, "/%s:1.%d", ent->d_name, 4);
    strcat(dir, subdir);
    if ((pDir = opendir(dir)) == NULL)  {  
        LOGE("Cannot open directory:%s/", dir);  
        return -ENODEV;  
    }
                       
    while ((ent = readdir(pDir)) != NULL) {
        //dbg_time("%s\n", ent->d_name);
        if (strncmp(ent->d_name, "usbmisc", strlen("usbmisc")) == 0) {
            strcpy(subdir, dir);
            strcat(subdir, "/usbmisc");
            if ((pSubDir = opendir(subdir)) == NULL)  {  
                LOGE("Cannot open directory:%s/", subdir);
                break;
            }
            while ((subent = readdir(pSubDir)) != NULL) {
                if (strncmp(subent->d_name, "cdc-wdm", strlen("cdc-wdm")) == 0) {
                    LOGD("Find qmichannel = %s", subent->d_name);
                    find_qmichannel = 1;
                    break;
                }                         
            }
            closedir(pSubDir);
        } 

        else if (strncmp(ent->d_name, "GobiQMI", strlen("GobiQMI")) == 0) {
            strcpy(subdir, dir);
            strcat(subdir, "/GobiQMI");
            if ((pSubDir = opendir(subdir)) == NULL)  {  
                LOGE("Cannot open directory:%s/", subdir);
                break;
            }
            while ((subent = readdir(pSubDir)) != NULL) {
                if (strncmp(subent->d_name, "qcqmi", strlen("qcqmi")) == 0) {
                    LOGD("Find qmichannel = %s", subent->d_name);
                    find_qmichannel = 1;
                    break;
                }                         
            }
            closedir(pSubDir);
        }         

        else if (strncmp(ent->d_name, "net", strlen("net")) == 0) {
            strcpy(subdir, dir);
            strcat(subdir, "/net");
            if ((pSubDir = opendir(subdir)) == NULL)  {  
                LOGE("Cannot open directory:%s/", subdir);
                break;
            }
            while ((subent = readdir(pSubDir)) != NULL) {
                if ((strncmp(subent->d_name, "wwan", strlen("wwan")) == 0)
                    || (strncmp(subent->d_name, "eth", strlen("eth")) == 0)
                    || (strncmp(subent->d_name, "usb", strlen("usb")) == 0)) {
                    *pp_usbnet_adapter = strdup(subent->d_name);
                    LOGD("Find usbnet_adapter = %s", *pp_usbnet_adapter );
                    break;
                }                         
            }
            closedir(pSubDir);
        } 

        if (find_qmichannel && *pp_usbnet_adapter)
            break;
    }
    closedir(pDir);     

    return (find_qmichannel && *pp_usbnet_adapter) ? 0 : -1;
}

static pid_t ql_ndis_pid = 0;
static int ql_ndis_quit = 0;
static pthread_t ql_ndis_thread;
static int ndis_create_thread(pthread_t * thread_id, void * thread_function, void * thread_function_arg ) {
    static pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    if (pthread_create(thread_id, &thread_attr, thread_function, thread_function_arg)!=0) {
        LOGE("%s %s errno: %d (%s)", __FILE__, __func__, errno, strerror(errno));
        return 1;
    }
    pthread_attr_destroy(&thread_attr); /* Not strictly necessary */
    return 0; //thread created successfully
}

static void ql_sleep(int sec) {
    int msec = sec * 1000;
    while (!ql_ndis_quit && (msec > 0)) {
        msec -= 200;
        usleep(200*1000);
    }
}

static void* ndis_thread_function(void*  arg) {
    char **argvv = (char **)arg;
    char *apn=NULL, *user=NULL, *password=NULL, *auth_type=NULL;
    const char *argv[10];
    int argc = 0;

    LOGD("%s enter", __func__);
        
    if (argvv[0] != NULL) apn=argvv[0];
    if (argvv[1] != NULL) user=argvv[1];
    if (argvv[2] != NULL) password=argvv[2];
    if (argvv[3] != NULL) auth_type=argvv[3];

    //LOGD("apn = %s", apn);
    //LOGD("user = %s", user);
    //LOGD("password = %s", password);
    //LOGD("auth_type = %s", auth_type);

    argv[argc++] = "quectel-CM";
    argv[argc++] = "-s";
    if (apn && apn[0])
        argv[argc++] = apn;
    else
         argv[argc++] = "\"\"";
    if (user && user[0])
        argv[argc++] = user;
    else
        argv[argc++] = "\"\"";
    if (password && password[0])
        argv[argc++] = password;
    else
        argv[argc++] = "\"\"";
    if (auth_type && auth_type[0])
        argv[argc++] = auth_type;
    else
        argv[argc++] = "\"\"";
    argv[argc] = NULL;   
        
    while (!ql_ndis_quit) {
        int child_pid = fork();
        if (child_pid == 0) {
            exit(quectel_CM(argc, (char**) argv));
        } else if (child_pid < 0) {
            LOGE("failed to start ('%s'): %s\n", "quectel-CM", strerror(errno));
            break;
        } else {
            int sleep_msec = 3000;
            int status, retval = 0;
            ql_ndis_pid = child_pid;
            waitpid(child_pid, &status, 0);
            ql_ndis_pid = 0;
            if (WIFSIGNALED(status)) {
                retval = WTERMSIG(status);
                LOGD("*** %s: Killed by signal %d retval = %d\n", "quectel-CM", WTERMSIG(status), retval);
            } else if (WIFEXITED(status) && WEXITSTATUS(status) > 0) {
                retval = WEXITSTATUS(status);
                LOGD("*** %s: Exit code %d retval = %d\n", "quectel-CM", WEXITSTATUS(status), retval);
            }
            ql_sleep(3);
        }
    }
        
    if (argvv[0] != NULL) free(argvv[0]);
    if (argvv[1] != NULL) free(argvv[1]);
    if (argvv[2] != NULL) free(argvv[2]);
    if (argvv[3] != NULL) free(argvv[3]);

    ql_ndis_thread = 0;
    LOGD("%s exit", __func__);
    pthread_exit(NULL);
    return NULL;     
}

static int ql_kill_pid(pid_t pid, int signo) {
    if (pid <= 1)
        return -EINVAL;  
    return kill(pid, signo);
}

int ql_ndis_stop(int signo);
int ql_ndis_start(const char *apn, const char *user, const char *password, const char *auth_type) {    
    static char *argv[4] = {NULL, NULL, NULL, NULL};
    ql_ndis_stop(SIGKILL);

    //LOGD("apn = %s", apn);
    //LOGD("user = %s", user);
    //LOGD("password = %s", password);
    //LOGD("auth_type = %s", auth_type);
    
    if (apn != NULL) argv[0] = strdup(apn);
    if (user != NULL) argv[1] = strdup(user);
    if (password != NULL) argv[2] = strdup(password);
    if (auth_type != NULL) argv[3] = strdup(auth_type);

    ql_ndis_quit = 0;
    if (!ndis_create_thread(&ql_ndis_thread, ndis_thread_function, (void*)(argv)))
        return getpid();
    else
        return -1;
}

int ql_ndis_stop(int signo) {
    int kill_time = 15;
    pid_t ndis_pid = ql_ndis_pid;
    ql_ndis_quit = 1;

    if ((ql_ndis_pid <= 0) && (ql_ndis_thread <= 0))
        return 0;
    
    if (ndis_pid > 0) {
        if (fork() == 0) {//kill may take long time, so do it in child process
            int kill_time = 10;
            ql_kill_pid(ndis_pid, signo);
            while(kill_time--&& !ql_kill_pid(ndis_pid, 0)) //wait pppd quit
                sleep(1);
            if ((signo != SIGKILL) && (kill_time < 0) && !ql_kill_pid(ndis_pid, 0))
                ql_kill_pid(ndis_pid, SIGKILL);
            exit(0);
        } 
    }
    
    while ((ql_ndis_thread > 0) && (kill_time-- > 0)) {
        sleep(1);
    }
    if (ql_ndis_pid > 0) {
        LOGD("%s force kill ql_ndis_pid", __func__);
        ql_kill_pid(ql_ndis_pid, SIGKILL);     
        ql_ndis_pid = 0;
    }
    if (ql_ndis_thread > 0) {
        LOGD("%s force kill ql_ndis_thread", __func__);
        pthread_kill(ql_ndis_thread, SIGKILL);
        ql_ndis_thread = 0;
    }

    LOGD("%s cost %d sec", __func__, 15 - kill_time );
    return 0;
}
