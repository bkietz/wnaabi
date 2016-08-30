# This module defines
#  SPHINX_EXECUTABLE
#  Sphinx_add_doc()

find_program(SPHINX_EXECUTABLE
  NAMES sphinx-build
  DOC "Sphinx documentation generator"
)

set(SPHINX_BUILDERS "dirhtml;xml" CACHE STRING "\
  What type(s) of documentation Sphinx should build.\
  Possible values:\
      html       to make standalone HTML files\
      dirhtml    to make HTML files named index.html in directories\
      singlehtml to make a single large HTML file\
      pickle     to make pickle files\
      json       to make JSON files\
      htmlhelp   to make HTML files and a HTML help project\
      qthelp     to make HTML files and a qthelp project\
      applehelp  to make an Apple Help Book\
      devhelp    to make HTML files and a Devhelp project\
      epub       to make an epub\
      epub3      to make an epub3\
      latex      to make LaTeX files, you can set PAPER=a4 or PAPER=letter\
      latexpdf   to make LaTeX files and run them through pdflatex\
      latexpdfja to make LaTeX files and run them through platex/dvipdfmx\
      text       to make text files\
      man        to make manual pages\
      texinfo    to make Texinfo files\
      info       to make Texinfo files and run them through makeinfo\
      gettext    to make PO message catalogs\
      changes    to make an overview of all changed/added/deprecated items\
      xml        to make Docutils-native XML files\
      pseudoxml  to make pseudoxml-XML files for display purposes\
      linkcheck  to check all external links for integrity\
      doctest    to run all doctests embedded in the documentation (if enabled)\
      coverage   to run coverage check of the documentation (if enabled)\
  ")

# create a target named ${target}
# which invokes sphinx-build with given builders
# build into directory ${target}
# read documentation from ${CMAKE_CURRENT_SOURCE_DIR}/${srcroot}
# assume that conf.py is at documentation root
function(sphinx_doc target srcroot)
  set(builddir "${CMAKE_CURRENT_BINARY_DIR}/${target}")
  set(root "${CMAKE_CURRENT_SOURCE_DIR}/${srcroot}")
  add_custom_target(${target}
    COMMAND "echo"
    COMMENT "Generate Sphinx documentation with ${SPHINX_BUILDERS}"
  )
  foreach(builder IN LISTS SPHINX_BUILDERS)
    set(subtarget "${target}_BUILDER_${builder}")
    add_custom_target(${subtarget} ALL
      COMMAND ${SPHINX_EXECUTABLE} -b ${builder}
      -c ${root}
      ${root}
      ${builddir}
      COMMENT "Sphinx documentation sub target, BUILDER=${builder}"
    )
    add_dependencies(${target} ${subtarget})
  endforeach()
endfunction()


