# syzygy-standard

**(WIP : this library is still in development; parts of features is uncompleted. (mac/linux support, basic support for unknown devices, ismessyflag, SIMD))**

syzygy-standard is a library which supports all operating devices (by default, it supports popular devices such as windows, mac, linux(x11, wayland) and it makes user able to write adaption code of its device.)
In addition, its performance has been enhanced with SIMD, ismessyflag(ignore update when any changes couldn't find.) and so on.
What is more, this library focused on user-experience. Users don't have to call setup function itself because this uses lazy-initialization and not only that, if you forgot to call popfont, the system calls popfont automatically/internally!
Furthermore, instance-creation, instance-destruction and shutdown can be written in a single line!
In detail, you don't have to call destroy-instance each instance. You can directly call shutdown if you want!
Lastly, this library is single-file. You can install this with little effort!
