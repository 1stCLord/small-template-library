# small-template-library
A C++ template library

API Reference:
root
pod_vector - a container class designed as a potential specialisation of std::vector for trivially copyable types
utf_convert - a class for easy conversion between wide strings and std::string by utilising utf8 encoding in std::string representations

threading
worker_thread_pool - a group of worker_threads for running workers
worker_thread - the thread that lets a worker perform arbitrary work
worker - an abstract class that can be inherited from to perform work on a worker_thread_pool
messenger - a worker that calls an arbitrary function on any registered listeners
message - a function and parameter wrapper, passed to a messenger and called on all it's listeners
simple_async - a futureless std::async alternative for calling a callable which returns no results on a task thread
thread_local_member - a wrapper around an object that can only be accessed while running on a specific thread to guarantee thread safety
