#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/types.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define die(str, args...)   \
    do {                    \
        perror(str);        \
        exit(EXIT_FAILURE); \
    } while (0)

typedef struct
{
    __u16 type;
    __u16 code;
    __s32 value;
    char* description;
} TEventData;

void set_bit(int fd, unsigned long int request, unsigned long int bit)
{
    if (ioctl(fd, request, bit) < 0)
        die("error: ioctl");
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("Requires one parameter..");

        return 0;
    }

    int productId = atoi(argv[1]);

    int fd;
    struct uinput_user_dev virtual_touch_device;
    struct input_event ev;
    int i;

    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0)
        die("error: open");

    set_bit(fd, UI_SET_EVBIT, EV_SYN);
    set_bit(fd, UI_SET_EVBIT, EV_KEY);
    set_bit(fd, UI_SET_KEYBIT, BTN_TOUCH);
    set_bit(fd, UI_SET_EVBIT, EV_ABS);
    set_bit(fd, UI_SET_ABSBIT, ABS_X);
    set_bit(fd, UI_SET_ABSBIT, ABS_Y);
    set_bit(fd, UI_SET_ABSBIT, ABS_MT_SLOT);
    set_bit(fd, UI_SET_ABSBIT, ABS_MT_POSITION_X);
    set_bit(fd, UI_SET_ABSBIT, ABS_MT_POSITION_Y);
    set_bit(fd, UI_SET_ABSBIT, ABS_MT_TRACKING_ID);
    set_bit(fd, UI_SET_PROPBIT, INPUT_PROP_DIRECT);

    memset(&virtual_touch_device, 0, sizeof(virtual_touch_device));
    snprintf(virtual_touch_device.name, UINPUT_MAX_NAME_SIZE, "Virtual Touch Device - %#x", productId);
    virtual_touch_device.id.bustype = BUS_VIRTUAL;
    virtual_touch_device.id.vendor = 0x453;
    virtual_touch_device.id.product = productId;
    virtual_touch_device.id.version = 1;

    virtual_touch_device.absmin[ABS_X] = 0;
    virtual_touch_device.absmax[ABS_X] = 1280;
    virtual_touch_device.absmin[ABS_Y] = 0;
    virtual_touch_device.absmax[ABS_Y] = 720;
    virtual_touch_device.absmin[ABS_MT_SLOT] = 0;
    virtual_touch_device.absmax[ABS_MT_SLOT] = 9;
    virtual_touch_device.absmin[ABS_MT_POSITION_X] = 0;
    virtual_touch_device.absmax[ABS_MT_POSITION_X] = 1280;
    virtual_touch_device.absmin[ABS_MT_POSITION_Y] = 0;
    virtual_touch_device.absmax[ABS_MT_POSITION_Y] = 720;
    virtual_touch_device.absmin[ABS_MT_TRACKING_ID] = 0;
    virtual_touch_device.absmax[ABS_MT_TRACKING_ID] = 65535;

    if (write(fd, &virtual_touch_device, sizeof(virtual_touch_device)) < 0)
        die("error: write");

    if (ioctl(fd, UI_DEV_CREATE) < 0)
        die("error: ioctl");

    printf("virtual touch device created [%s]...\r\n", virtual_touch_device.name);

    scanf("%d", &i);

    const int cnt = 20;
    TEventData eventData[cnt] = {
        {EV_ABS, ABS_MT_TRACKING_ID, 160, "EV_ABS-ABS_MT_TRACKING_ID"},

        {EV_ABS, ABS_MT_POSITION_X, 400, "EV_ABS-ABS_MT_POSITION_X"},
        {EV_ABS, ABS_MT_POSITION_Y, 400, "EV_ABS-ABS_MT_POSITION_Y"},
        {EV_KEY, BTN_TOUCH, 1, "EV_KEY-BTN_TOUCH"},
        {EV_ABS, ABS_X, 400, "EV_ABS-ABS_X"},
        {EV_ABS, ABS_Y, 400, "EV_ABS-ABS_Y"},
        {EV_SYN, 0, 0, "EV_SYN"},

        {EV_ABS, ABS_MT_POSITION_X, 450, "EV_ABS-ABS_MT_POSITION_X"},
        {EV_ABS, ABS_MT_POSITION_Y, 450, "EV_ABS-ABS_MT_POSITION_Y"},
        {EV_ABS, ABS_X, 450, "EV_ABS-ABS_X"},
        {EV_ABS, ABS_Y, 450, "EV_ABS-ABS_Y"},
        {EV_SYN, 0, 0, "EV_SYN"},


        {EV_ABS, ABS_MT_POSITION_X, 500, "EV_ABS-ABS_MT_POSITION_X"},
        {EV_ABS, ABS_MT_POSITION_Y, 500, "EV_ABS-ABS_MT_POSITION_Y"},
        {EV_ABS, ABS_X, 500, "EV_ABS-ABS_X"},
        {EV_ABS, ABS_Y, 500, "EV_ABS-ABS_Y"},
        {EV_SYN, 0, 0, "EV_SYN"},

        {EV_ABS, ABS_MT_TRACKING_ID, -1, "EV_ABS-ABS_MT_TRACKING_ID"},
        {EV_KEY, BTN_TOUCH, 0, "EV_KEY-BTN_TOUCH"},
        {EV_SYN, 0, 0, "EV_SYN"}};

    char j;
    printf("press any key to start sending events....");
    scanf("%c", &j);

    sleep(2);
    for (i = 0; i < cnt; i++) {
        memset(&ev, 0, sizeof(struct input_event));
        ev.type = eventData[i].type;
        ev.code = eventData[i].code;
        ev.value = eventData[i].value;

        if (write(fd, &ev, sizeof(struct input_event)) < 0) {
            printf("failed %s =>", eventData[i].description);
            die("error: write ");
            printf("\r\n");
        } else {
            printf("success %s \r\n", eventData[i].description);
        }
    }

    printf("simulation complete...Press any key to destory..\r\n");

    scanf("%c", &j);

    if (ioctl(fd, UI_DEV_DESTROY) < 0)
        die("error: ioctl");

    close(fd);

    printf("destroyed...\r\n");

    return 0;
}
