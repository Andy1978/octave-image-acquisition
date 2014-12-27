run ("../PKG_ADD");
addpath (canonicalize_file_name ("../inst"));
system (sprintf ("xfce4-terminal --command 'gdb -p %d'", getpid ()), "async");

#vi = __v4l2_handler_open__("/dev/video0");
