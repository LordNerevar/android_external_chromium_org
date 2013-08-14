{
  'TOOLS': ['newlib', 'glibc', 'pnacl'],
  'SEARCH': [
    '.',
    'pepper',
    '../third_party/newlib-extras',
    'include',
  ],
  'TARGETS': [
    {
      'NAME' : 'nacl_io',
      'TYPE' : 'lib',
      'SOURCES' : [
        'dbgprint.c',
        "event_emitter.cc",
        "event_listener.cc",
        "h_errno.cc",
        "host_resolver.cc",
        "kernel_handle.cc",
        "kernel_intercept.cc",
        "kernel_object.cc",
        "kernel_proxy.cc",
        "kernel_wrap_glibc.cc",
        "kernel_wrap_newlib.cc",
        "kernel_wrap_win.cc",
        "mount.cc",
        "mount_dev.cc",
        "mount_html5fs.cc",
        "mount_http.cc",
        "mount_mem.cc",
        "mount_node.cc",
        "mount_node_dir.cc",
        "mount_node_html5fs.cc",
        "mount_node_http.cc",
        "mount_node_mem.cc",
        "mount_node_tty.cc",
        "mount_passthrough.cc",
        "nacl_io.cc",
        "path.cc",
        "pepper_interface.cc",
        "real_pepper_interface.cc",
        "syscalls/accept.c",
        "syscalls/access.c",
        "syscalls/bind.c",
        "syscalls/chdir.c",
        "syscalls/chmod.c",
        "syscalls/chown.c",
        "syscalls/connect.c",
        "syscalls/fchown.c",
        "syscalls/fsync.c",
        "syscalls/ftruncate.c",
        "syscalls/getcwd.c",
        "syscalls/getdents.c",
        "syscalls/gethostbyname.c",
        "syscalls/getpeername.c",
        "syscalls/getsockname.c",
        "syscalls/getsockopt.c",
        "syscalls/getwd.c",
        "syscalls/herror.c",
        "syscalls/hstrerror.cc",
        "syscalls/htonl.c",
        "syscalls/htons.c",
        "syscalls/inet_ntoa.cc",
        "syscalls/inet_ntop.cc",
        "syscalls/ioctl.c",
        "syscalls/isatty.c",
        "syscalls/lchown.c",
        "syscalls/link.c",
        "syscalls/listen.c",
        "syscalls/mkdir.c",
        "syscalls/mount.c",
        "syscalls/ntohl.c",
        "syscalls/ntohs.c",
        "syscalls/poll.c",
        "syscalls/rmdir.c",
        "syscalls/recv.c",
        "syscalls/recvfrom.c",
        "syscalls/recvmsg.c",
        "syscalls/remove.c",
        "syscalls/tcflush.c",
        "syscalls/tcgetattr.c",
        "syscalls/tcsetattr.c",
        "syscalls/select.c",
        "syscalls/send.c",
        "syscalls/sendmsg.c",
        "syscalls/sendto.c",
        "syscalls/setsockopt.c",
        "syscalls/shutdown.c",
        "syscalls/socket.c",
        "syscalls/socketpair.c",
        "syscalls/unlink.c",
        "syscalls/umount.c",
        "syscalls/uname.c",
        "syscalls/utime.c",
      ],
    }
  ],
  'HEADERS': [
    {
      'FILES': [
        'dbgprint.h',
        "event_emitter.h",
        "event_listener.h",
        "error.h",
        "host_resolver.h",
        "inode_pool.h",
        "ioctl.h",
        "kernel_handle.h",
        "kernel_intercept.h",
        "kernel_object.h",
        "kernel_proxy.h",
        "kernel_wrap.h",
        "kernel_wrap_real.h",
        "mount.h",
        "mount_dev.h",
        "mount_factory.h",
        "mount_html5fs.h",
        "mount_http.h",
        "mount_mem.h",
        "mount_node.h",
        "mount_node_char.h",
        "mount_node_dir.h",
        "mount_node_html5fs.h",
        "mount_node_http.h",
        "mount_node_mem.h",
        "mount_node_tty.h",
        "mount_passthrough.h",
        "nacl_io.h",
        "osdirent.h",
        "osinttypes.h",
        "osmman.h",
        "ossocket.h",
        "osstat.h",
        "ostime.h",
        "ostypes.h",
        "osunistd.h",
        "osutime.h",
        "ostermios.h",
        "path.h",
        "pepper_interface.h",
        "real_pepper_interface.h",
        "typed_mount_factory.h",
      ],
      'DEST': 'include/nacl_io',
    },
    {
      'FILES': [
        "arpa/inet.h",
        "netdb.h",
        "netinet/in.h",
        "netinet/tcp.h",
        "netinet6/in6.h",
        "poll.h",
        "sys/mount.h",
        "sys/select.h",
        "sys/socket.h",
        "sys/termios.h",
        "sys/utsname.h",
      ],
      'DEST': 'include/newlib',
    },
    {
      'FILES': [
        "arpa/inet.h",
        "netdb.h",
        "netinet/in.h",
        "netinet/tcp.h",
        "netinet6/in6.h",
        "poll.h",
        "sys/mount.h",
        "sys/select.h",
        "sys/socket.h",
        "sys/termios.h",
        "sys/utsname.h",
      ],
      'DEST': 'include/pnacl',
    },
    {
      'FILES': [
        "poll.h",
      ],
      'DEST': 'include/win',
    },
    {
      'FILES': [
        "all_interfaces.h",
        "define_empty_macros.h",
        "undef_macros.h",
      ],
      'DEST': 'include/nacl_io/pepper',
    }
  ],
  'DEST': 'src',
  'NAME': 'nacl_io',
}
