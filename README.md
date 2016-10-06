# Concurrent Wrapper (conwrap)


##Preface

Consider a simple sync method:
```c++
class Dummy
{
	public:
		Result syncMethod(int param)
		{
			// do some actula work and generate result
			return Result();
		}
};
```

To call this method asynchronously one can use `std::async` helper template:
```c++
// creating an object dummy
Dummy dummy;

// invoking syncMethod asynchronously
std::future<Result> resultFuture = std::async(
	std::launch::async,                  // invocation policy
	[=](int param) mutable -> Result     // lambda that defines task's funtionality
	{
		return dummy.syncMethod(param);  // invoking syncMethod method from the submitted task
	},
	123                                  // param value that will be passed to the submitted task
);
```

However `std::async` is not a good fit for task-based processing. On the other hand Concurrent Wrapper provides functionality to invoke arbitrary code asynchronously in a task-based processing fashion:
```c++
// creating a concurrent wrapper object that containg an intance of Dummy
conwrap::ProcessorQueue<Dummy> processor;

// submitting an asynchronous task that will invoke syncMethod
std::future<Result> resultFuture = processor.process([param = 123](auto context)
{
	// gaining access to the object dummy
	auto dummyPtr = context.getResource();

	// invoking syncMethod method from the submitted task
	return dummyPtr->syncMethod(param);
});
```


##Background

If you are into C++ asynchronous code then you recall great talk by Herb Sutter: [“C++ and Beyond 2012: Herb Sutter - C++ Concurrency”](https://channel9.msdn.com/Shows/Going+Deep/C-and-Beyond-2012-Herb-Sutter-Concurrency-and-Parallelism). Otherwise I highly recommend watching it as Herb presents a pattern for a generic wrapper. This pattern can be used to turn synchronous code into asynchronous one in a so elegant way that it cannot be wrong. However there is a catch: presented design is too simplistic and omits one crucial feature: task (handler) should be able to issue a new task (handler). Why is this feature so crucial and why it ruins elegance of the presented design?

If you are building an application with very simple logic of the tasks that you submit for asynchronous execution (like for example log library where a task simply outputs a message to all sinks) then mentioned feature is irrelevant. But in most cases logic presented in the tasks is more complex and requires new tasks to be issued based on certain conditions. For example if you develop a server using [Boos.Asio]( http://www.boost.org/doc/libs/1_61_0/doc/html/boost_asio.html) then your onReceive task (handler) will submit a new task (handler) to keep receiving new data coming in.

Now why submitting a new task from a running tasks spoils all the elegance? Without this functionality processing termination and processing flushing are straight-forward to implement:
- To terminate processing one just need to add a new task that will tell processor to stop. This can be done safely from the processor’s destructor which will wait for that task to complete (see Herb’s talk for details).
- If you want to flush then one just need to submit an empty task and wait when it’s complete; it would mean all the tasks before empty one were processed.

Neither termination nor queue flush works as described above if tasks may submit a new task. Indeed:
- Once termination task is submitted, there might be pending tasks in the queue, and if it submitts a new task so termination task will not be the last one.
- Once flush task is submitted, pending task may submit a new task, which will be executed after flush task completes; this contradicts to flush semantic.

Wouldn’t it be nice to have it all: asynchronous wrapper turning synchronous code into asynchronous tasks (just like Herb presented) with possibility to submit a new task from running task, with possibility to destroy processing safely at any time, with possibility to flush pending tasks to make sure there are no dangling pointers… Well, this is what this library does for you ;)


##How it works

The goal of this library is the same as for std::async – to run arbitrary code asynchronously, however semantic is different. std::async spawns a new thread every time it is called. This approach is not suitable for many cases because OS thread creation is a heavy operation; besides this approach does not ensure sequential invocation of submitted tasks. Concurrent Wrapper uses a single thread for all submitted tasks, which are processed sequentially. Basically it is an implementation of “multiple providers / single consumer” pattern.

The fact that a task being executed may submit new tasks causes extra complexity. For example, a task may still be executed when processor’s destructor is called hence processor is unavailable for accepting a new task. This is resolved by Concurrent Wrapper with a use of a proxy processor object which is passed for tasks to be used for issuing a new tasks.

One important feature of Concurrent Wrapper is the possibility to flush task execution queue. Below there is an example demonstrating why this feature is crucial:
```c++
// creating an object on the heap
auto objectPtr = std::make_unique<SomeClass>();

conwrap::ProcessorQueue<Dummy> processor();

// submitting an asynchronous task that will invoke syncMethod
processor.process([capturedPtr = objectPtr.get()]
{
	// here pointer to the object can be used including for passing to any sub-sequent task
	// note that waiting for this particular task to complete is not safe enough because it may pass capturedPtr sub-sequent tasks
});

// without this flush operation capturedPtr becomes a dangling pointer after object is deleted
processor.flush();

// deleting object
objectPtr.reset();
```


In the similar way, Concurrent Wrapper ensures there is no dangling pointers left when its destructor is called by waiting for all pending tasks to complete. This is a different semantic compared to Boost.Asio, which requires stop method to be called and leaves unfinished handlers.

So far, conwrap::ProcessorQueue was used to demonstrate Concurrent Wrapper's functionality. There is a similar class available called conwrap::ProcessorAsio. This class provides possibility to use Boos.Asio for processing arbitrary code asynchronously. This is very useful in case of asynchronous TCP/UDP servers based on Boost.Asio. Basically this class provides possibility to combine boost handlers with arbitrary code submitted for execution in the task-based processing fashion. Really cool thing about conwrap::ProcessorAsio is that it has the same semantic as conwrap::ProcessorQueue which means you can flush Boost.Asio handlers and delete processor object safely.


##Usage

... 