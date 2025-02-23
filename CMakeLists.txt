cmake_minimum_required(VERSION 3.10)
project(zsim)

set(CMAKE_CXX_STANDARD 14)

include_directories(misc/hooks)
include_directories(src)
include_directories(src/g_std)
include_directories(src/HATS)
include_directories(src/virt)

add_executable(zsim
        cpp_test/instruction.cpp
        cpp_test/multi_thread.cpp
        cpp_test/test.cpp
        cpp_test/thread.cpp
        misc/hooks/fortran_hooks.c
        misc/hooks/test.c
        misc/hooks/test.cpp
        misc/hooks/zsim_hooks.h
        misc/hooks/zsim_jni.cpp
        src/g_heap/dlmalloc.h.c
        src/g_std/g_list.h
        src/g_std/g_multimap.h
        src/g_std/g_string.h
        src/g_std/g_unordered_map.h
        src/g_std/g_unordered_set.h
        src/g_std/g_vector.h
        src/g_std/stl_galloc.h
        src/HATS/BDFS.h
        src/HATS/Edge.h
        src/HATS/VO.h
        src/virt/common.h
        src/virt/cpu.cpp
        src/virt/fs.cpp
        src/virt/patchdefs.h
        src/virt/port_virtualizer.h
        src/virt/ports.cpp
        src/virt/syscall_name.h
        src/virt/time.cpp
        src/virt/time_conv.h
        src/virt/timeout.cpp
        src/virt/virt.cpp
        src/virt/virt.h
        src/access_tracing.cpp
        src/access_tracing.h
        src/barrier.h
        src/bithacks.h
        src/breakdown_stats.h
        src/cache.cpp
        src/cache.h
        src/cache_arrays.cpp
        src/cache_arrays.h
        src/coherence_ctrls.cpp
        src/coherence_ctrls.h
        src/config.cpp
        src/config.h
        src/constants.h
        src/contention_sim.cpp
        src/contention_sim.h
        src/core.h
        src/core_recorder.cpp
        src/core_recorder.h
        src/cpuenum.h
        src/cpuid.h
        src/ddr_mem.cpp
        src/ddr_mem.h
        src/debug.h
        src/debug_harness.cpp
        src/debug_harness.h
        src/debug_zsim.cpp
        src/debug_zsim.h
        src/decoder.cpp
        src/decoder.h
        src/detailed_mem.cpp
        src/detailed_mem.h
        src/detailed_mem_params.cpp
        src/detailed_mem_params.h
        src/dramsim_mem_ctrl.cpp
        src/dramsim_mem_ctrl.h
        src/dumptrace.cpp
        src/event_queue.h
        src/event_recorder.h
        src/fftoggle.cpp
        src/filter_cache.h
        src/galloc.cpp
        src/galloc.h
        src/gmutex.h
        src/hash.cpp
        src/hash.h
        src/hdf5_stats.cpp
        src/ideal_arrays.h
        src/init.cpp
        src/init.h
        src/intrusive_list.h
        src/locks.h
        src/log.cpp
        src/log.h
        src/lookahead.cpp
        src/mem_ctrls.cpp
        src/mem_ctrls.h
        src/memory_hierarchy.cpp
        src/memory_hierarchy.h
        src/monitor.cpp
        src/mtrand.h
        src/mutex.h
        src/network.cpp
        src/network.h
        src/null_core.cpp
        src/null_core.h
        src/ooo_core.cpp
        src/ooo_core.h
        src/ooo_core_recorder.cpp
        src/ooo_core_recorder.h
        src/pad.h
        src/parse_vdso.cpp
        src/part_repl_policies.h
        src/partition_mapper.cpp
        src/partition_mapper.h
        src/partitioner.h
        src/pin_cmd.cpp
        src/pin_cmd.h
        src/prefetcher.cpp
        src/prefetcher.h
        src/prio_queue.h
        src/proc_stats.cpp
        src/proc_stats.h
        src/process_stats.cpp
        src/process_stats.h
        src/process_tree.cpp
        src/process_tree.h
        src/profile_stats.h
        src/rdtsc.h
        src/repl_policies.h
        src/scheduler.cpp
        src/scheduler.h
        src/simple_core.cpp
        src/simple_core.h
        src/slab_alloc.h
        src/sorttrace.cpp
        src/stats.h
        src/stats_filter.cpp
        src/stats_filter.h
        src/str.h
        src/text_stats.cpp
        src/tick_event.h
        src/timing_cache.cpp
        src/timing_cache.h
        src/timing_core.cpp
        src/timing_core.h
        src/timing_event.cpp
        src/timing_event.h
        src/trace_driver.cpp
        src/trace_driver.h
        src/tracing_cache.cpp
        src/tracing_cache.h
        src/utility_monitor.cpp
        src/utility_monitor.h
        src/weave_md1_mem.h
        src/zsim.cpp
        src/zsim.h
        src/zsim_harness.cpp)
