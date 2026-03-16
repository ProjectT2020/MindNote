# Alt + o: open external resources
## pdf / jpeg / png in local filesystem
If the underlying node represents a relative path to a pdf/jpeg/png etc. file,
Alt + o will open it with firefox.

Examples:
```code
resources/jls24.pdf
resources/screenshot-1.png
```

## wiki term
If the underlying node's parent has text `wiki`,
Alt + o will open this term by simply concat a prefix
`https://en.wikipedia.org/wiki/World` to form a complet URL

Example:
```code
Universe
```
will become
```code
https://en.wikipedia.org/wiki/Universe
```

### custom wiki
You can also specify a custom wiki prefix to connect to your own wiki system
by adding a context metadata `wiki_prefix`.
see [context metadata] for more info about how to use metadata and how does it works.

Example:
hit Alt + o on node `Universe`
```code
│   ╭ B
│   │   ╭ .meta ─ wiki_prefix ─ http://192.168.1.100:1234/content/wikipedia_en_all_maxi_2026-02/
├ A ┴ C ┤   ╭ b
│       ╰ a ┴ c ┬ d
│               ╰ wiki ─ Universe
```
will open the following link in firefox
```code
http://192.168.1.100:1234/content/wikipedia_en_all_maxi_2026-02/Universe
```

## regular URL
if the underlying node denotes a regular HTTP url, it will be open with the browser.