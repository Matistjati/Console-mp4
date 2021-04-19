from PIL import ImageFont, ImageDraw, Image

textDensity = []

fontsize = 14
txt = ""
for i in range(32, 127):
    txt += chr(i)

#for i in range(161, 255):
#    txt += chr(i)
#print(txt)

for char in txt:
    image = Image.new("RGB", (8,15), (255,255,255))
    draw = ImageDraw.Draw(image)
    font = ImageFont.truetype("Consola.ttf", fontsize)

    draw.text((0, 0), char, (0,0,0), font=font)

    count = 0
    width, height = image.size
    for x in range(width):
        for y in range(height):
            r,g,b = image.getpixel((x,y))
            if r < 253:
                count += 1
    
    textDensity.append((float(count)/(8*15.), char))

#print(textDensity)
textDensity = sorted(textDensity, key=lambda f: f[0])
#print(textDensity)
textDensity = [(v*(1.45) if v != 1 else 1, ch) for v, ch in textDensity]
[print(f"{{{v}, \'{ch}\'}},") for v, ch in textDensity]
print(len(textDensity))
