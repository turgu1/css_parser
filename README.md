# A simple CSS class and CSS Parser in C++

(Still not ready yet)

This is a simple, single-source code, CSS Parser written in C++. It implements lexical and syntax analysis, ideal for someone who wants to add its own semantic processing. It is based on the grammar as defined [here](https://www.w3.org/TR/CSS21/grammar.html) and was coded by hand.

A CSS class containing the parsed structures demonstrates how the parser can be used as a internal CSS generator. The CSS structure contains only a subset of the whole CSS capability and could be enhanced easily using the current implementation as a source of reference.

The following are the main differences from the 2.1 grammar:

- No support for Unicode
- Supports numbers starting with '.'
- Supports @font-style 
- Added Length number types: vh, vz, vmin, vmax, rem, ch

- First cut of the @media extension (not used)

The main objective is to have a combined CSS class and CSS parser suitable to include in an EPUB reader that fits on very small reader devices with
no more than 4 MBytes of available memory and supply "good enough" formatting of pages.

## What remains to be done:

- @import implementation in the CSS parser
- testing

## CSS class - Selector limitations:

- Supports selection nodes containing tags, classes, ids
- Supports :first-child qualifier
- Supports the following operators combinaing selectin nodes: adjacent, child and descendant

## CSS class - Properties supported:

- display
- font-family
- font-size
- font-style
- font-weight
- height
- line-height
- margin
- margin-bottom
- margin-left
- margin-right
- margin-top
- src
- text-align
- text-indent
- text-transform
- width
