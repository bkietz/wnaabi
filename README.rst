======
wnaabi
======

Who Needs an ABI?
-----------------

.. image:: https://readthedocs.org/projects/wnaabi/badge/?version=latest
   :target: https://wnaabi.readthedocs.io
.. image:: https://travis-ci.org/bkietz/wnaabi.svg
   :target: https://travis-ci.org/bkietz/wnaabi
.. image:: https://ci.appveyor.com/api/projects/status/9o5hogg7afbvfmoi?svg=true
   :target: https://ci.appveyor.com/project/BenjaminKietzman/wnaabi

This project presents a nifty implementation of ``type_index``
with some features you will love:

- Canonical typename format consistent across all compilers
- Access to typename **tokens** so you can hate on my canonical
  format however you want.
- Efficient creation, hashing, and typename retrieval- most of the
  work is done at compile time and this project doesn't even use
  the keyword ``virtual``.

The premise of this project is that the C++ type system, generalized
``constexpr`` functions, and the rock-solid and minimalist C ABI can
bootstrap a pretty awesome subset of the things a C++ ABI would be used for.

.. code-block:: c++

  #include <cassert>
  #include <wnaabi/type_info.hpp>

  using namespace wnaabi;
  namespace bar { struct baz; }

  int main()
  {
    assert(type_info<bar::baz>::name_tokens(runtime_visitors::stringify_t{}).str == "bar::baz");
  }
