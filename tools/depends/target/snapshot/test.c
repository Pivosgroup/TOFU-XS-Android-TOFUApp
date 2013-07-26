#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>        /* low-level i/o */
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>

//#include <linux/pivos/aml_snapshot.h>

#define AMSNAPSHOT_IOC_MAGIC 'T'
#define AMSNAPSHOT_IOC_SET_CONFIG  _IOW(AMSNAPSHOT_IOC_MAGIC, 0x02, struct aml_snapshot_t)
#define AMSNAPSHOT_IOC_GET_CONFIG  _IOR(AMSNAPSHOT_IOC_MAGIC, 0x03, struct aml_snapshot_t)
#define AMSNAPSHOT_IOC_GET_FRAME   _IOW(AMSNAPSHOT_IOC_MAGIC, 0x04, struct aml_snapshot_t)

#define AMSNAPSHOT_FOURCC(a, b, c, d)\
	((__u32)(a) | ((__u32)(b) << 8) | ((__u32)(c) << 16) | ((__u32)(d) << 24))

#define AMSNAPSHOT_FMT_S24_BGR   AMSNAPSHOT_FOURCC('B', 'G', 'R', '3') /* 24  BGR-8-8-8     */
#define AMSNAPSHOT_FMT_S24_RGB   AMSNAPSHOT_FOURCC('R', 'G', 'B', '3') /* 24  RGB-8-8-8     */
#define AMSNAPSHOT_FMT_S32_RGBA  AMSNAPSHOT_FOURCC('R', 'G', 'B', 'A') /* 32  BGR-8-8-8-8   */
#define AMSNAPSHOT_FMT_S32_BGRA  AMSNAPSHOT_FOURCC('B', 'G', 'R', 'A') /* 32  BGR-8-8-8-8   */
#define AMSNAPSHOT_FMT_S32_ABGR  AMSNAPSHOT_FOURCC('A', 'B', 'G', 'R') /* 32  BGR-8-8-8-8   */
#define AMSNAPSHOT_FMT_S32_ARGB  AMSNAPSHOT_FOURCC('A', 'R', 'G', 'B') /* 32  BGR-8-8-8-8   */

struct aml_snapshot_t {
  unsigned int  src_x;
  unsigned int  src_y;
  unsigned int  src_width;
  unsigned int  src_height;
  unsigned int  dst_width;
  unsigned int  dst_height;
  unsigned int  dst_stride;
  unsigned int  dst_format;
  unsigned int  dst_size;
  unsigned long dst_vaddr;
};

static void errno_exit(const char *s) {
  fprintf (stderr, "%s error %d, %s\n", s, errno, strerror (errno));

  exit(EXIT_FAILURE);
}

static int xioctl(int fd, int request, void *arg) {
  int r;
  do r = ioctl(fd, request, arg);
  while (-1 == r && EINTR == errno);

  return r;
}

static int fd = -1;
static struct aml_snapshot_t aml_snapshot = {0};

static void snap_frame(char *dev_name,
  int src_x, int src_y, int src_w, int src_h, int dst_w, int dst_h)
{
  // default src bounds
  aml_snapshot.src_x      = src_x;
  aml_snapshot.src_y      = src_y;
  aml_snapshot.src_width  = src_w;
  aml_snapshot.src_height = src_h;

  aml_snapshot.dst_width  = dst_w;
  aml_snapshot.dst_height = dst_h;
  aml_snapshot.dst_stride = dst_w * 3;
  aml_snapshot.dst_size   = dst_w * 3 * dst_h;
  if (aml_snapshot.dst_vaddr)
    free((void*)aml_snapshot.dst_vaddr);
  aml_snapshot.dst_vaddr = (unsigned long)calloc(aml_snapshot.dst_size, 1);

  if (xioctl(fd, AMSNAPSHOT_IOC_GET_FRAME, &aml_snapshot) == -1) {
    errno_exit("AMSNAPSHOT_IOC_GET_FRAME");
  }

  //writing to standard output
  write(STDOUT_FILENO, (void*)aml_snapshot.dst_vaddr, aml_snapshot.dst_size);
}

static void close_device(void) {
  if (-1 == close(fd))
    errno_exit("close");

  fd = -1;
}

static void open_device(char *dev_name) {
  struct stat st;

  if (-1 == stat(dev_name, &st)) {
    fprintf(stderr, "Cannot identify '%s': %d, %s\n",
       dev_name, errno, strerror (errno));
    exit(EXIT_FAILURE);
  }

  if (!S_ISCHR(st.st_mode)) {
    fprintf(stderr, "%s is no device\n", dev_name);
    exit (EXIT_FAILURE);
  }

  fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);

  if (-1 == fd) {
    fprintf(stderr, "Cannot open '%s': %d, %s\n",
       dev_name, errno, strerror (errno));
    exit (EXIT_FAILURE);
  }
}

#define VIDEO_PATH       "/dev/amvideo"
#define AMSTREAM_IOC_MAGIC  'S'
#define AMSTREAM_IOC_GET_VIDEO_AXIS _IOR(AMSTREAM_IOC_MAGIC, 0x4b, unsigned long)
int amvideo_utils_get_position(int *x, int *y, int *w, int *h)
{
  int video_fd;
  int axis[4];

  video_fd = open(VIDEO_PATH, O_RDWR);
  if (video_fd < 0) {
    return -1;
  }

  ioctl(video_fd, AMSTREAM_IOC_GET_VIDEO_AXIS, &axis[0]);
  close(video_fd);

  *x = axis[0];
  *y = axis[1];
  *w = axis[2] - axis[0] + 1;
  *h = axis[3] - axis[1] + 1;

  return 0;
}

int main(int argc, char **argv) {
  char *dev_name = "/dev/aml_snapshot";

  int dst_w, dst_h;
#if 0
  dst_w = 1280, dst_h = 720;
#else
  //dst_w = 160,  dst_h = 160;
  dst_w = 640,  dst_h = 360;
#endif
  int src_x = 0, src_y = 0, src_w = 0, src_h = 0;

  // match source ratio if possible
  amvideo_utils_get_position(&src_x, &src_y, &src_w, &src_h);
  if (src_w > 0) {
    float ratio = (float)src_h / (float)src_w;
    dst_h = (float)dst_w * ratio;
  }

  fprintf(stderr, "'%s': "
    "src_x(%d), src_y(%d), src_w(%d), src_h(%d), dst_w(%d), dst_h(%d)\n",
    dev_name, src_x, src_y, src_w, src_h, dst_w, dst_h);

  open_device(dev_name);
  snap_frame (dev_name, src_x, src_y, src_w, src_h, dst_w, dst_h);
  close_device();

  return 0;
}

