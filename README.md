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
	123);                                // param value that will be passed to the submitted task
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

If you are into C++ asynchronous code then you recall great talk by Herb Sutter: [“C++ and Beyond 2012: Herb Sutter - C++ Concurrency”](https://channel9.msdn.com/Shows/Going+Deep/C-and-Beyond-2012-Herb-Sutter-Concurrency-and-Parallelism). Otherwise I highly recommend watching it as Herb presents a pattern for a generic wrapper. This pattern can be used to turn synchronous code into asynchronous one in a so elegant way that it cannot be wrong. However there is a catch: presented design is too simplistic and omits one crucial feature: task (handler) should be able to issue a new task (handler). Why is this feature so crucial and why it ruins elegance of presented design?

If you are building an application with very simple logic of the tasks that you submit for asynchronous execution (like for example log library where a task simply outputs a message to all sinks) then mentioned feature is irrelevant. But in most cases logic presented in the tasks is more complex and requires new tasks to be issued based on certain conditions. For example if you develop a server using [Boos.Asio]( http://www.boost.org/doc/libs/1_61_0/doc/html/boost_asio.html) then your onReceive task (handler) will submit a new task (handler) to keep receiving new data coming in.

Now why submitting a new task from a running tasks spoils elegance. Without this functionality terminating processing and flushing processing queue are straight-forward to implement:
- To terminate processing one just need to add a new task that will tell processor to stop. This can be safely done from the processor’s destructor which will wait for that task to complete (see Herb’s talk for details).
- If you want to flush then one just need to submit an empty task and wait when it’s complete; it would mean all tasks before empty one were processed.
