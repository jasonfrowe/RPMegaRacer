-- RP6502 Tilemap Exporter
local spr = app.activeSprite
if not spr then return print("No active sprite") end

local layer = app.activeLayer
if not layer.isTilemap then 
    return app.alert("Select a TILEMAP layer first!") 
end

local cel = layer:cel(app.activeFrame)
local img = cel.image
local width = img.width   -- Should be 64
local height = img.height -- Should be 48

-- Prompt for save location
local dlg = Dialog("Export RP6502 Map")
dlg:file{ id="export_file", label="Save as:", save=true, filename="track_map.bin" }
dlg:button{ id="ok", text="Export" }
dlg:button{ id="cancel", text="Cancel" }
dlg:show()

if dlg.data.ok then
    local f = io.open(dlg.data.export_file, "wb")
    
    -- Iterate through the tilemap pixels (indices)
    -- Aseprite stores tilemap data in the 'pixels' of a Tilemap layer
    for y = 0, height - 1 do
        for x = 0, width - 1 do
            local pixel = img:getPixel(x, y)
            -- app.pixelColor.tileI extracts the index from the internal data
            local tileIndex = app.pixelColor.tileI(pixel)
            
            -- Write as a single byte (0-255)
            f:write(string.char(tileIndex & 0xFF))
        end
    end
    
    f:close()
    app.alert("Exported " .. (width*height) .. " bytes to " .. dlg.data.export_file)
end
