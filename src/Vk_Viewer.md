### Semaphore and Fence description

**Note**: read this if you forget again what Semaphores and Fences are used for in Vulkan XD.

Source: https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Rendering_and_presentation

#### Semaphore

**In Vulkan, Semaphores are used to order execution on the GPU**

A semaphore is used to add order between queue operations. Queue operations refer to the work we submit to a queue, either in a command buffer or from within a function as we will see later. Examples of queues are the graphics queue and the presentation queue. Semaphores are used both to order work inside the same queue and between different queues.

There happens to be two kinds of semaphores in Vulkan, binary and timeline. Because only binary semaphores will be used in this tutorial, we will not discuss timeline semaphores. Further mention of the term semaphore exclusively refers to binary semaphores.

A semaphore is either unsignaled or signaled. It begins life as unsignaled. The way we use a semaphore to order queue operations is by providing the same semaphore as a 'signal' semaphore in one queue operation and as a 'wait' semaphore in another queue operation. For example, lets say we have semaphore S and queue operations A and B that we want to execute in order. What we tell Vulkan is that operation A will 'signal' semaphore S when it finishes executing, and operation B will 'wait' on semaphore S before it begins executing. When operation A finishes, semaphore S will be signaled, while operation B wont start until S is signaled. After operation B begins executing, semaphore S is automatically reset back to being unsignaled, allowing it to be used again.

Pseudo-code of what was just described:
```C++
    VkCommandBuffer A, B = ... // record command buffers
    VkSemaphore S = ... // create a semaphore

    // enqueue A, signal S when done - starts executing immediately
    vkQueueSubmit(work: A, signal: S, wait: None)

    // enqueue B, wait on S to start
    vkQueueSubmit(work: B, signal: None, wait: S)
```

Note that in this code snippet, both calls to vkQueueSubmit() return immediately - the waiting only happens on the GPU. The CPU continues running without blocking. To make the CPU wait, we need Fences.

#### Fences

**In Vulkan, Fences are used to order the waiting process of GPU operations on the CPU**

A fence has a similar purpose, in that it is used to synchronize execution, but it is for ordering the execution on the CPU, otherwise known as the host. Simply put, if the host needs to know when the GPU has finished something, we use a fence.

Similar to semaphores, fences are either in a signaled or unsignaled state. Whenever we submit work to execute, we can attach a fence to that work. When the work is finished, the fence will be signaled. Then we can make the host wait for the fence to be signaled, guaranteeing that the work has finished before the host continues.

A concrete example is taking a screenshot. Say we have already done the necessary work on the GPU. Now need to transfer the image from the GPU over to the host and then save the memory to a file. We have command buffer A which executes the transfer and fence F. We submit command buffer A with fence F, then immediately tell the host to wait for F to signal. This causes the host to block until command buffer A finishes execution. Thus we are safe to let the host save the file to disk, as the memory transfer has completed.

Pseudo-code for what was described:
```C++
    VkCommandBuffer A = ... // record command buffer with the transfer
    VkFence F = ... // create the fence

    // enqueue A, start work immediately, signal F when done
    vkQueueSubmit(work: A, fence: F)

    vkWaitForFence(F) // blocks execution until A has finished executing

    save_screenshot_to_disk() // can't run until the transfer has finished
```

Unlike the semaphore example, this example does block host execution. This means the host won't do anything except wait until execution has finished. For this case, we had to make sure the transfer was complete before we could save the screenshot to disk.

In general, it is preferable to not block the host unless necessary. We want to feed the GPU and the host with useful work to do. Waiting on fences to signal is not useful work. Thus we prefer semaphores, or other synchronization primitives not yet covered, to synchronize our work.

Fences must be reset manually to put them back into the unsignaled state. This is because fences are used to control the execution of the host, and so the host gets to decide when to reset the fence. Contrast this to semaphores which are used to order work on the GPU without the host being involved.

In summary, semaphores are used to specify the execution order of operations on the GPU while fences are used to keep the CPU and GPU in sync with each-other.