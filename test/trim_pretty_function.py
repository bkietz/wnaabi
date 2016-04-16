"""
This is essentially `grep`.

Search a compiled binary for typenames
and check their surrounding printable characters.
If the compiler successfully optimized away
__PRETTY_FUNCTION__'s prefix and suffix, they
will not appear in the binary.
"""

import sys
import re
import unittest
import os

binary_path = sys.argv.pop()
binary_file = open(binary_path, 'rb')
binary_str = binary_file.read()
typenames = [ 'foo', 'bar::baz', 'bar::quux', 'bar::tpl', 'bar::a_tpl']

def get_prefix_suffix():
  """
  Assume there is one type named cruft_crib
  on which wnaabi::pretty_function::c_str<> is used without
  trimming, and the embedded strings !!!cruft_suffix=1!!!
  !!!cruft_strlen=48!! (where 1 and 48 are the actual lengths
  of the suffix and the whole string respectively)
  to allow capture of the prefix and suffix.
  """
  cruft_strlen = re.search(r'!!!cruft_strlen=(\d+)!!!', binary_str)
  cruft_strlen = int(cruft_strlen.group(1))

  cruft_suffix = re.search(r'!!!cruft_suffix=(\d+)!!!', binary_str)
  cruft_suffix = int(cruft_suffix.group(1))

  cruft_prefix = cruft_strlen - cruft_suffix - len('cruft_crib')

  return re.search(
    '([ -~]{%d})cruft_crib([ -~]{%d})' % (cruft_prefix, cruft_suffix),
    binary_str).groups()

class PrettyFunctionTrimTest(unittest.TestCase):

  def test_prefix_suffix_trimmed(self):
    prefix, suffix = get_prefix_suffix()
    #print "prefix '''%s'''\nsuffix '''%s'''" % (prefix, suffix)
    pattern = ('(.{%d})(' + '|'.join(typenames) + ')(.{%d})'
      ) % (len(prefix), len(suffix))

    for match in re.finditer(pattern, binary_str):
      maybe_prefix, typename, maybe_suffix = match.groups()
      self.assertNotEqual(maybe_prefix, prefix)
      self.assertNotEqual(maybe_suffix, suffix)

if __name__ == "__main__":
  unittest.main()
