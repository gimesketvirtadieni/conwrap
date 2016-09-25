Concurrent Wrapper (conwrap)
============================


Consider a simple sync method:

class Dummy
{
  public:
    Result syncMethod(int param)
    {
      return Result();
    }
};


To call this method asynchronously one can use std::async helper template:

Dummy dummy;
std::future<DummyResult> resultFuture = std::async(std::launch::async, [=](int param) mutable -> DummyResult
{
  return dummy.syncMethod(param);
}, 123 /* param value */);


However std::async is not a good fit for task-based processing. CONWRAP provides an alternative to std::async in a following way:

conwrap::ProcessorQueue<Dummy> processor;
std::future<DummyResult> resultFuture = processor.process([param = 123](auto context)
{
  auto dummyPtr = context.getResource();
  return dummyPtr->syncMethod(param);
});

