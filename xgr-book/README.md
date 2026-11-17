# xgr::book v2: each market gets its own book implementation

clang 18 or newer: `clang++ -std=c++23 -O2 -Iinclude -I. example.cpp`

include/xgr/book/
  facade.hpp                  __facade CPO base
  core.hpp                    interfaces only: message queries, write verbs, read-side concept
  algo.hpp                    OPTIONAL: generic verbs built from 4 primitives, plus rules and knobs
  impl/fifo_array_book.hpp    CN-style: price-limit array, intrusive lists, id-indexed nodes; every verb hand-written
  impl/vector_level_book.hpp  ASX/ASX24-style: per-level vectors for mid-queue inserts; primitives only
markets/                      cn / asx / asx24: messages + `using book = ...`
example_mixed.cpp             a book that hand-writes its hot verb and takes the rest from algo.hpp

How a verb resolves for book B:
  1. B hand-writes tag_invoke(verb, B&, Msg)   -> that is used (most specialized)
  2. otherwise, if B has the primitives        -> the generic verb from algo.hpp
  3. otherwise                                 -> book_for<B, Msg> is false (compile-time)
