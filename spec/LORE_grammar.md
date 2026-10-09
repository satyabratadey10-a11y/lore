# LORE Language Grammar (v0.1)

LORE = a controlled English-like language for UI + native logic.
Compiles: `.lore` → LORE compiler → LLVM IR → `.so` (and optionally `.wasm`).

## 1. Lexical rules

```ebnf
letter        = "a".."z" | "A".."Z" | "_" ;
digit         = "0".."9" ;
ident         = letter , { letter | digit | "-" } ;
number        = digit , { digit } , [ "." , digit , { digit } ] ;
unit          = "dp" | "sp" | "px" | "%" ;
dimension     = number , unit ;
string        = '"' , { any_char_except_quote | "{" , ident , "}" } , '"' ;
hex_colour    = "#" , hex , hex , hex , hex , hex , hex ;
colour_ref    = "CS:" , ident ;
comment       = "///" , { any_char_except_newline } ;
```

- Whitespace and newlines are ignored (except inside strings).
- Keywords are case-sensitive. `EndOf...` and `.end...att.` are single tokens.
- `{name}` inside a string inserts a state value (string interpolation).
- Filler words `a`, `the` may be skipped by the lexer (optional feature).

## 2. Program structure

```ebnf
program        = package_header ,
                 { import_stmt } ,
                 [ permission_block ] ,
                 [ hccs_block ] ,
                 root_decl ,
                 { top_item } ;

top_item       = element_decl | attr_stmt | attr_end | state_decl | handler ;
```

## 3. Package header

```ebnf
package_header = package_field , package_field , package_field ;
package_field  = "package:" , ( "name" | "id" | "label" ) , ":" , string , "." ;
```

> Note: your original used `lable`. The grammar uses `label`; the lexer may
> accept `lable` as an alias if you want to keep it.

## 4. Imports

```ebnf
import_stmt    = [ "from" , "[" , ident , "]" ] , "import" , "[" , ident , "]" , "." ;
```

## 5. Permission block

```ebnf
permission_block = "permission" , { perm_line } , "EndOfPermission." ;
perm_line        = perm_item , { "and" , perm_item } , "." ;
perm_item        = ident , ":" , perm_value ;
perm_value       = "true" | "false" | number ;
```

Meaning of each permission:

| Permission | Meaning |
|---|---|
| `InputHarness : true` | The `.so` can ask for input itself when it is run with no input supplied. |
| `eachtimevalue : 1` | Each input event takes exactly **one** input signal (file, number, or any type). |
| `ShowDisplay : true` | UI is drawn on a canvas all the time (not tied to a timeframe). |
| `OutputDisplay : true` | UI is shown only at specific moments: during input, during the process, or at the end to give output only. |

Rules:
- `ShowDisplay` and `OutputDisplay` both need a canvas.
- `ShowDisplay : true` already includes `OutputDisplay`, so writing both is
  unnecessary (the compiler gives a warning, not an error).
- Without `ShowDisplay` and without `OutputDisplay`, the `.so` has no UI
  (headless mode, e.g. a pure engine library).

## 6. Colour shortcuts (HCCS)

```ebnf
hccs_block     = "HCCS" , { hccs_line } , "EndOfHCCS." ;
hccs_line      = string , "is" , string , "." ;       (* "#4287f5" is "blue". *)
```

Use later as `CS:blue`. Compiler checks that every `CS:` name exists.

## 7. UI elements

```ebnf
root_decl      = "root" , "is" , element_kind , "." ;

element_decl   = "put" , element_kind , ident , "inside" , ident , "." ;

element_kind   = "bg" | "text" | "button" | "image" | "row" | "column" | "box" ;

attr_stmt      = ident , attr_name , [ "is" ] , value , "." ;
attr_name      = ident , [ ":" , ident ] ;            (* image  |  image:pressed *)

attr_end       = ".end" , ident , "att." ;            (* .endbgatt.  .endtitleatt. *)

value          = number | dimension | string | hex_colour | colour_ref
               | keyword | expr ;
keyword        = "fullscreen" | "cover" | "contain" | "stretch" | "tile"
               | "center" | "start" | "end" | "wrap" ;
```

Rules:
- `root is bg.` declares the root; its handle is `bg`.
- `put button play inside bg.` declares a child; its handle is `play`.
- Attribute lines start with the **handle**: `play text is "Go".`
- `.end<handle>att.` closes that element's attribute list. The handle in the
  closing token must match (`.endplayatt.`), otherwise compile error.
- `is` is optional in attribute lines: `bg size fullscreen.` = `bg size is fullscreen.`

## 8. Image textures

Any element may use an image file as its look.

| Attribute | Meaning | Example |
|---|---|---|
| `image` | Main texture / background image | `bg image is "assets/wall.png".` |
| `image:pressed` | Texture while pressed (buttons) | `play image:pressed is "assets/down.png".` |
| `image:disabled` | Texture when disabled | `play image:disabled is "assets/off.png".` |
| `image-fit` | `cover`, `contain`, `stretch`, `tile` | `bg image-fit is cover.` |
| `image-tint` | Colour multiplied over the image | `play image-tint is CS:red.` |

```ebnf
image_path     = string ;      (* must end in .png .jpg .jpeg .webp *)
```

Compiler checks:
1. File exists at compile time (error if missing).
2. Extension is one of `.png`, `.jpg`, `.jpeg`, `.webp`.
3. Image is packed into the output as a resource table (or copied beside the
   `.so`), so the host app can load it with the UI.

## 9. State, logic, events

```ebnf
state_decl     = "state" , ident , "is" , expr , "." ;

handler        = "when" , ident , "is" , event_name , { statement } , "EndOfWhen." ;
event_name     = "tapped" | "pressed" | "released" | "changed" | "input" ;

statement      = assign_stmt | if_stmt | repeat_stmt | call_stmt ;
assign_stmt    = ident , "is" , expr , "." ;
call_stmt      = ( "add" , expr , "to" , ident
                 | "subtract" , expr , "from" , ident
                 | "show" , expr
                 | "play" , ident ) , "." ;

if_stmt        = "if" , condition , "then" , { statement } ,
                 [ "otherwise" , { statement } ] , "EndOfIf." ;
repeat_stmt    = "repeat" , expr , "times" , { statement } , "EndOfRepeat." ;
```

## 10. Expressions

```ebnf
expr           = term , { ( "plus" | "minus" ) , term } ;
term           = factor , { ( "times" | "divided" "by" ) , factor } ;
factor         = number | string | ident | colour_ref | "(" , expr , ")" ;

condition      = expr , comparator , expr , { ( "and" | "or" ) , condition } ;
comparator     = "is" "equal" "to" | "is" "greater" "than" | "is" "less" "than"
               | "is" "not" ;
```

Important: `is` alone **assigns**; comparisons always use the longer forms
(`is equal to`, `is greater than`, ...). That removes the ambiguity.

## 11. Block terminator summary

| Construct | Closes with |
|---|---|
| `permission` | `EndOfPermission.` |
| `HCCS` | `EndOfHCCS.` |
| `when ...` | `EndOfWhen.` |
| `if ...` | `EndOfIf.` |
| `repeat ...` | `EndOfRepeat.` |
| UI element attributes | `.end<handle>att.` |

## 12. Compiler stages (build order)

1. Lexer (tokens above)
2. Parser (this grammar) → AST
3. Checker (handles exist, `CS:` names exist, image files exist, types)
4. LLVM IR emitter → `clang -shared -fPIC` → `libapp.so`
5. Runtime library (C++) that draws `bg`, `text`, `button`, `image` and loads textures

## 13. Exported C ABI of the generated `.so`

```c
void ore_init(const char* config);     // permissions + root
void ore_input(const void* signal);    // one signal per call (eachtimevalue : 1)
void ore_render(void* surface);        // draw the UI
```
(Rename to `lore_*` when you finalize the prefix.)
