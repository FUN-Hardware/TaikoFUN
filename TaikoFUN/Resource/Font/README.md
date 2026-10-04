# Illustrator text assets

SVG sources: komarifun711-pixel/TaikoFUN, PRs FUN-Hardware/TaikoFUN#10 and #11.
Source commits: 58d0bd475c73d49adfa6da12fc12363688797b8f (01-10),
ee05396a58e608bf242bcf69ed3f1d94dd0abdd6 (11-30).

Actual title mapping, verified from the artwork:

| Title | Black SVG | White SVG/PNG |
| --- | --- | --- |
| シャイニングスター | 01 | 06 |
| 8OROCHI(裏) | 02 | 07 |
| ドンカマ2000 | 03 | 08 |
| 万戈イムー一ノ十(成仏2000) | 04 | 09 |
| 続・〆ドレー2000 | 05 | 10 |

Numbers 1-9, then 0: black 11-20; white 21-30.
Names use `asset_XX.svg` / `asset_XX.png`. All SVG text is outlined.

Only the selected white assets have game PNGs. PNGs are transparent RGBA,
128 pixels high. They were rendered with ImageMagick from a temporary SVG
root with height 128 and width round(viewBoxWidth / viewBoxHeight * 128),
keeping the original viewBox and path data, using `magick -background none
temporary.svg PNG32:asset_XX.png`. Source SVGs retain their original viewBox.
The game scales these images to the current font height and preserves aspect
ratio. Titles replace existing title text; digits replace the existing roll
hit count. SkinData loads the textures once, and PlayScene draws them.
