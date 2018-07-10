: dump-font ( --)
        256 32 - 32 / 0 do 32 0 do j 5 lshift 32 + i + emit loop cr loop cr ;

.( active font glyphs)cr cr
dump-font
