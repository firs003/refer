

---- linxj 2014-05-29 ---
整理了工程，添加了simulator，确保在linux和windows能同时编译通过
关于linux和windows的编程差异，请阅读aboutos.txt
工程说明
所有库的文件都在libsrc下
gcc下调试linux工程，make 和 make lib 都可以
vis_mmnpapi.h 为对外接口，main.c需要包含他
vis_mmnp.h 为对内接口，库里的文件都将用到他
vis_os.h 为何操作系统有关的头文件