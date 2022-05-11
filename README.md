# Tiny thread pool

Lightweight header-only thread pool with the most essential features. Tasks can be coupled with a callback that will be invoked on the main thread at a suitable time. It's also possible to utilize std::future to await tasks.

## Example

```cpp
// created somewhere in your application
ttp::ThreadPool thread_pool(4);

// ...

thread_pool.Enqueue([](){
    CreateMeshes()
}, [](){
    // Perform potential callback on the 
    // main thread when work is complete
    UploadToGPU();
});

// In your game/application loop (if using callbacks)
while (game_is_running_) {
    thread_pool.ExecuteCallbacks();
    // update & render game
}
```
