-- NetHack 3.6	tower.des	$NHDT-Date: 1432512784 2015/05/25 00:13:04 $  $NHDT-Branch: master $:$NHDT-Revision: 1.9 $
--	Copyright (c) 1989 by Jean-Christophe Collet
-- NetHack may be freely redistributed.  See license for details.
--
--
-- Upper stage of Vlad's tower
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "solidify")
des.map({ halign = "half-left", valign = "center", map = [[
  --- --- ---  
  |.| |.| |.|  
---S---S---S---
|.......+.+...|
---+-----.-----
  |...\.|.+.|  
---+-----.-----
|.......+.+...|
---S---S---S---
  |.| |.| |.|  
  --- --- ---  
]] });

local niches = { {03,01}, {03,09}, {07,01}, {07,09}, {11,01}, {11,09} };
shuffle(niches);

des.ladder("down", 11,05)
-- The lord and his court
des.monster("Vlad the Impaler", 06, 05)
des.monster("V",niches[1])
des.monster("V",niches[2])
des.monster("V",niches[3])
des.monster("V",niches[4])
des.monster("V",niches[5])
des.monster("V",niches[6])
-- The doors
des.door("closed",08,03)
des.door("closed",10,03)
des.door({ state = "locked", x=03, y=04, iron=1 })
des.door({ state = "locked", x=10, y=05, iron=1 })
des.door("locked",08,07)
des.door("locked",10,07)
des.door({ state = "locked", x=03, y=06, iron=1 })
-- treasures
des.object({ id = "chest", coord = {07,05},
             contents = function()
                des.object({ class = "/", id = "wishing", spe = 1 })
             end
});

des.object("chest",niches[6])
des.object("chest",niches[1])
des.object("chest",niches[2])
des.object("chest",niches[3])
des.object({ id = "chest", coord=niches[4],
             contents = function()
                des.object({ id = "wax candle", quantity=d(4,2) })
             end
});
des.object({ id = "chest", coord=niches[5],
             contents = function()
                des.object({ id = "tallow candle", quantity=d(4,2) })
             end
});
-- We have to protect the tower against outside attacks
des.non_diggable(selection.area(00,00,14,10))
