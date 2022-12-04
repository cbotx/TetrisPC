#ifndef __DLL_INTERFACE_H__
#define __DLL_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif
    __declspec(dllexport) void __stdcall __halt__();

    __declspec(dllexport) void __stdcall __core_init__(const char path[]);

    __declspec(dllexport) void __stdcall __destroy__();

    __declspec(dllexport) void __stdcall __calculate_best_move__(int pc_idx,
                                                                 const int pieces[],
                                                                 const int bag_used,
                                                                 const int field[],
                                                                 int depth,
                                                                 int& ori,
                                                                 int& x,
                                                                 int& y);

#ifdef __cplusplus
}
#endif

#endif