# Subtitle Manager (subman)
This command line tool will help you out in:

- Merging two or more subtitles into one
- Adding styles (colors, fonts, ...)
- Fixing time issues

## Help:
```
Usage: subman command [input-files...] [args...]
SubMan (Subtitle Manager):
-h [ --help ]                      Show this help page.
-i [ --input-files ] arg           Input files
-f [ --force ]                     Force writing on existing files.
-o [ --output ] arg                Output file path
-r [ --recursive ]                 Recursively looking for input files.
--merge-method arg (=top2bottom)   The style of merge method.
Values:
top2bottom
bottom2top
left2right
right2left
-m [ --merge ]                     Merge subtitles into one subtitle
-s [ --styles ] arg                space-separated styles for each inputs;
separate each input by comma.
e.g: normal, italic red, bold #00ff00
-e [ --output-format ] arg (=auto) Output format
--style                            It's command that will let you style the
input file with the help of --style or
change the timings with --timing and etc.
-v [ --verbose ]
-t [ --timing ] arg                space-separed timing commands for each
inputs; separate each input by comma.
e.g: gap:100ms
e.g: shift:2s
--override
--command arg (=help)              the command. possible values: append,
help, merge, search, style
-c [ --contains ] arg              Search for subtitles that contain the
specified values.
-m [ --matches ] arg               Filter the results to those subtitles that
match the specified values.
--regex arg                        Filter the results bases on those
subtitles that match the specified regular
expressions.
```

## Examples

```
subman merge -i en.srt spa.srt -s orangered,white -fo merged.srt
```

