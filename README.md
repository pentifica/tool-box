# tool-box
Collection of useful generic c++ classes
## SkipList
Implements a probablistic skip list data structure as described in [^1] and is based the work published in [^2].
[^1]: https://en.wikipedia.org/wiki/Skip_list#:~:text=The%20expected%20number,against%20storage%20costs
[^2]: https://rowjee.com/blog/skiplists

## StrSwitch
Implements the logic for implementing a switch statement using quoted strings and std::string. The code is based on the information presented in [^3]
[^3]: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function 

## Factory
Implements a fast object factory that allocates objects from a pre-allocated cache and returns the allocated objects to the cache when the objects are released. The cache will grow as needed.

## PrintTuple
A template for streaming an arbitrary tuple of values. All values must be streamable.

## RAII
Implements RAII for an encapsulated set of actions. The action must support copy semantics.