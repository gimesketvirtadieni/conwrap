# Concurrent Wrapper (conwrap)


##Preface

Consider a simple sync method:
```c++
class Dummy
{
	public:
		Result syncMethod(int param)
		{
			return Result();
		}
};
```

To call this method asynchronously one can use `std::async` helper template:
```c++
Dummy dummy;
std::future<Result> resultFuture = std::async(std::launch::async, [=](int param) mutable -> Result
{
	return dummy.syncMethod(param);
}, /* param value */ 123);
```

However `std::async` is not a good fit for task-based processing. On the other hand Concurrent Wrapper provides functionality to invoke arbitrary code asynchronously in a task-based processing fashion:
```c++
conwrap::ProcessorQueue<Dummy> processor;
std::future<Result> resultFuture = processor.process([param = 123](auto context)
{
	auto dummyPtr = context.getResource();
	return dummyPtr->syncMethod(param);
});
```


##Background

If you are into C++ asynchronous code then you recall great talk by Herb Sutter: [“C++ and Beyond 2012: Herb Sutter - C++ Concurrency”](https://channel9.msdn.com/Shows/Going+Deep/C-and-Beyond-2012-Herb-Sutter-Concurrency-and-Parallelism). Otherwise I highly recommend watching it as Herb presents a pattern for a generic wrapper. This pattern can be used to turn synchronous code into asynchronous one in a so elegant way that it cannot be wrong. However there is a catch: presented design is too simplistic and omits one crucial feature: task (handler) should be able to issue a new task (handler). Why is feature so crucial and why it ruins elegance of presented design?

If you are building an application with very simple logic of the tasks that you submit for asynchronous execution (like for example log library where a task simply outputs a message to all sinks) then mentioned feature is irrelevant. But in most cases logic presented in the tasks is more complex and requires new tasks to be issued based on certain conditions. For example if you develop a server using [Boos.Asio]( http://www.boost.org/doc/libs/1_61_0/doc/html/boost_asio.html) then your onReceive task (handler) will submit a new task (handler) to keep receive new data coming in.
