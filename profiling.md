# DigitalRF profiling

**NOTE**: In this document, `r45304a3` refers to the 'stalled-cycles-backend' hardware event.


## `perf record -a -e cycles,r45304a3 ./benchmark_rf_write_hdf5`

Both event measures report the kernel symbol `copy_user_enhanced_fast_string` to consume a lot of events:

* 14.46% of `cycles` events
* 25.25% of `stalled-cycles-backend` events

