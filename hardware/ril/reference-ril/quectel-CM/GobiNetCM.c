#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <stdio.h>
#include <ctype.h>
#include "QMIThread.h"

// IOCTL to generate a client ID for this service type
#define IOCTL_QMI_GET_SERVICE_FILE 0x8BE0 + 1

// IOCTL to get the VIDPID of the device
#define IOCTL_QMI_GET_DEVICE_VIDPID 0x8BE0 + 2

// IOCTL to get the MEID of the device
#define IOCTL_QMI_GET_DEVICE_MEID 0x8BE0 + 3

int GobiNetSendQMI(PQCQMIMSG pRequest) {
    int ret, fd;

    switch(pRequest->QMIHdr.QMIType) {
        case QMUX_TYPE_WDS: fd = clientWDS; break;
        case QMUX_TYPE_DMS: fd = clientDMS; break;
        case QMUX_TYPE_NAS: fd = clientNAS; break;
        case QMUX_TYPE_WDS_ADMIN: fd = clientWDA; break;
        default: fd = -1; break;
    }

    if (fd == -1) {
        dbg_time("%s QMIType: %d has no clientID", __func__, pRequest->QMIHdr.QMIType);
        return -ENODEV;
    }
    
    // Always ready to write
    if (1 == 1) {
        ssize_t nwrites = le16_to_cpu(pRequest->QMIHdr.Length) + 1 - sizeof(QCQMI_HDR);
        ret = write(fd, &pRequest->MUXMsg, nwrites);
        if (ret == nwrites) {
            ret = 0;
        } else {
            dbg_time("%s write=%d, errno: %d (%s)", __func__, ret, errno, strerror(errno));
        }
    } else {
        dbg_time("%s poll=%d, errno: %d (%s)", __func__, ret, errno, strerror(errno));
    }
    
    return ret;
}

static int GobiNetGetClientID(const char *qmichannel, UCHAR QMIType) {
    int ClientId;
    ClientId = open(qmichannel, O_RDWR | O_NONBLOCK | O_NOCTTY);
    if (ClientId == -1) {
        dbg_time("failed to open %s, errno: %d (%s)", qmichannel, errno, strerror(errno));
        return -1;
    }
    if (ioctl(ClientId, IOCTL_QMI_GET_SERVICE_FILE, QMIType) != 0) {
        dbg_time("failed to get ClientID for 0x%02x errno: %d (%s)", QMIType, errno, strerror(errno));
        close(ClientId);
        ClientId = -1;
    }

    switch (QMIType) {
        case QMUX_TYPE_WDS: dbg_time("Get clientWDS = %d", ClientId); break;
        case QMUX_TYPE_DMS: dbg_time("Get clientDMS = %d", ClientId); break;
        case QMUX_TYPE_NAS: dbg_time("Get clientNAS = %d", ClientId); break;
        case QMUX_TYPE_WDS_ADMIN: dbg_time("Get clientWDA = %d", ClientId); break;
        default: close(ClientId); ClientId = -1; break;
    }
    return ClientId;
}

void * GobiNetThread(void *pData) {
    clientWDS = GobiNetGetClientID(qmichannel, QMUX_TYPE_WDS);
    clientDMS = GobiNetGetClientID(qmichannel, QMUX_TYPE_DMS);
    clientNAS = GobiNetGetClientID(qmichannel, QMUX_TYPE_NAS);
    clientWDA = GobiNetGetClientID(qmichannel, QMUX_TYPE_WDS_ADMIN);

    //donot check clientWDA, there is only one client for WDA, if quectel-CM is killed by SIGKILL, i cannot get client ID for WDA again!
    if ((clientWDS == -1) || (clientDMS == -1) || (clientNAS == -1) /*|| (clientWDA == -1)*/) {
        if (clientWDS != -1) close(clientWDS);
        if (clientDMS != -1) close(clientDMS);
        if (clientNAS != -1) close(clientNAS);
        if (clientWDA != -1) close(clientWDA);
        dbg_time("%s Failed to open %s, errno: %d (%s)", __func__, qmichannel, errno, strerror(errno));
        qmidevice_send_event_to_main(RIL_INDICATE_DEVICE_DISCONNECTED);
        pthread_exit(NULL);
        return NULL;
    }
    
    qmidevice_send_event_to_main(RIL_INDICATE_DEVICE_CONNECTED);

    while (1) {
        struct pollfd pollfds[] = {{qmidevice_control_fd[1], POLLIN, 0}, {clientWDS, POLLIN, 0},
            {clientDMS, POLLIN, 0}, {clientNAS, POLLIN, 0}, {clientWDA, POLLIN, 0}};
        int ne, ret, nevents = sizeof(pollfds)/sizeof(pollfds[0]);

        do {
            ret = poll(pollfds, nevents, -1);
         } while ((ret < 0) && (errno == EINTR));
           
        if (ret <= 0) {
            dbg_time("%s poll=%d, errno: %d (%s)", __func__, ret, errno, strerror(errno));
            break;
        } 

        for (ne = 0; ne < nevents; ne++) {
            int fd = pollfds[ne].fd;
            short revents = pollfds[ne].revents;

            if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
                dbg_time("%s poll err/hup/inval", __func__);
                dbg_time("epoll fd = %d, events = 0x%04x", fd, revents);
                if (fd == qmidevice_control_fd[1]) {                
                } else {
                }
                if (revents & (POLLERR | POLLHUP | POLLNVAL))
                    goto __GobiNetThread_quit;
            } 

            if ((revents & POLLIN) == 0)
                continue;

            if (fd == qmidevice_control_fd[1]) {
                int triger_event;
                if (read(fd, &triger_event, sizeof(triger_event)) == sizeof(triger_event)) {
                    //DBG("triger_event = 0x%x", triger_event);
                    switch (triger_event) {
                        case RIL_REQUEST_QUIT:
                            goto __GobiNetThread_quit;
                        break;
                        case SIGTERM:
                        case SIGHUP:
                        case SIGINT:
                            QmiThreadRecvQMI(NULL);
                        break;                            
                        default:
                        break;
                    }
                }
            }

            if ((clientWDS == fd) || (clientDMS == fd) || (clientNAS == fd) || (clientWDA == fd)) {
                ssize_t nreads;
                UCHAR QMIBuf[512];
                PQCQMIMSG pResponse = (PQCQMIMSG)QMIBuf;
                
                nreads = read(fd, &pResponse->MUXMsg, sizeof(QMIBuf) - sizeof(QCQMI_HDR));
                if (nreads <= 0) {
                    dbg_time("%s read=%d errno: %d (%s)",  __func__, (int)nreads, errno, strerror(errno));
                    break;
                } 

                if (fd == clientWDS) pResponse->QMIHdr.QMIType = QMUX_TYPE_WDS;
                else if (fd  == clientDMS) pResponse->QMIHdr.QMIType = QMUX_TYPE_DMS;
                else if (fd  == clientNAS) pResponse->QMIHdr.QMIType = QMUX_TYPE_NAS;
                else if (fd  == clientWDA) pResponse->QMIHdr.QMIType = QMUX_TYPE_WDS_ADMIN;
                else continue;;

                pResponse->QMIHdr.IFType = USB_CTL_MSG_TYPE_QMI;
                pResponse->QMIHdr.Length = cpu_to_le16(nreads + sizeof(QCQMI_HDR)  - 1);
                pResponse->QMIHdr.CtlFlags = 0x00;
                pResponse->QMIHdr.ClientId = fd & 0xFF;

                QmiThreadRecvQMI(pResponse);
            }   
        }
    }

__GobiNetThread_quit:
    if (clientWDS != -1) { close(clientWDS);  clientWDS = -1; };
    if (clientDMS != -1) { close(clientDMS);  clientDMS = -1; };
    if (clientNAS != -1) { close(clientNAS);  clientNAS = -1; };
    if (clientWDA != -1) { close(clientWDA);  clientWDA = -1; };
    qmidevice_send_event_to_main(RIL_INDICATE_DEVICE_DISCONNECTED);
    QmiThreadRecvQMI(NULL); //main thread may pending on QmiThreadSendQMI()
    dbg_time("%s exit", __func__);
    pthread_exit(NULL);
    return NULL;
}
