#!/usr/bin/env python3
import io
import math
import os
import sys
import xml.etree.ElementTree as ET

from PIL import Image, ImageDraw
from svgpathtools import parse_path


THRESHOLD = 128


def svg_to_image(svg_path, width, height):
    with open(svg_path, "rb") as f:
        svg_data = f.read()

    try:
        import cairosvg

        png_bytes = cairosvg.svg2png(
            bytestring=svg_data,
            output_width=width,
            output_height=height,
        )
        img = Image.open(io.BytesIO(png_bytes))
        return img.convert("RGBA")
    except Exception:
        # Fallback path for environments without native Cairo.
        root = ET.fromstring(svg_data)
        view_box = root.attrib.get("viewBox", "").replace(",", " ").split()
        if len(view_box) == 4:
            src_x = float(view_box[0])
            src_y = float(view_box[1])
            src_w = float(view_box[2])
            src_h = float(view_box[3])
        else:
            src_x = 0.0
            src_y = 0.0
            src_w = float(root.attrib.get("width", width))
            src_h = float(root.attrib.get("height", height))

        scale_x = width / src_w
        scale_y = height / src_h

        def map_point(point):
            return ((point.real - src_x) * scale_x, (point.imag - src_y) * scale_y)

        img = Image.new("RGBA", (width, height), (255, 255, 255, 255))
        draw = ImageDraw.Draw(img)

        for element in root.iter():
            if not element.tag.endswith("path"):
                continue
            d = element.attrib.get("d")
            if not d:
                continue

            path = parse_path(d)
            current_points = []
            previous_end = None

            for segment in path:
                start = segment.start
                if previous_end is not None and abs(start - previous_end) > 1e-6:
                    if len(current_points) >= 3:
                        draw.polygon(current_points, fill=(0, 0, 0, 255))
                    current_points = []

                length = max(segment.length(error=1e-3), 1.0)
                samples = max(8, int(math.ceil(length / 1.5)))
                for i in range(samples + 1):
                    point = segment.point(i / samples)
                    mapped = map_point(point)
                    if not current_points or current_points[-1] != mapped:
                        current_points.append(mapped)
                previous_end = segment.end

            if len(current_points) >= 3:
                draw.polygon(current_points, fill=(0, 0, 0, 255))

        return img


def load_image(path, width, height):
    ext = os.path.splitext(path)[1].lower()
    if ext == ".svg":
        img = svg_to_image(path, width, height)
    else:
        img = Image.open(path)
        img = img.convert("RGBA")
        img = img.resize((width, height), Image.LANCZOS)
        # Flatten alpha: paste on white background.
        background = Image.new("RGBA", img.size, (255, 255, 255, 255))
        background.paste(img, mask=img.split()[3])
        img = background

    # Rotate 90 degrees counterclockwise.
    img = img.rotate(90, expand=True)
    return img


def image_to_c_array(img, array_name):
    # Convert to grayscale, then threshold to get white=1, black=0.
    img = img.convert("L")
    width, height = img.size
    pixels = list(img.get_flattened_data())
    packed = []
    for y in range(height):
        for x in range(0, width, 8):
            byte = 0
            for b in range(8):
                if x + b < width:
                    v = pixels[y * width + x + b]
                    bit = 1 if v >= THRESHOLD else 0
                    byte |= bit << (7 - b)
            packed.append(byte)

    c = "#pragma once\n#include <cstdint>\n\n"
    c += f"// size: {width}x{height}\n"
    c += f"static const uint8_t {array_name}[] = {{\n    "
    for i, v in enumerate(packed):
        c += f"0x{v:02X}, "
        if (i + 1) % 16 == 0:
            c += "\n    "
    c = c.rstrip(", \n") + "\n};\n"
    return c


def main():
    if len(sys.argv) < 5:
        print("Usage: python convert_image.py input.png output_name width height")
        sys.exit(1)

    input_path, output_name, width, height = sys.argv[1:5]
    array_name = output_name.capitalize() + "Icon"
    width, height = int(width), int(height)
    img = load_image(input_path, width, height)
    c_array = image_to_c_array(img, array_name)

    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    output_dir = os.path.join(project_root, "src", "components", "icons")
    os.makedirs(output_dir, exist_ok=True)
    output_path = os.path.join(output_dir, f"{output_name}.h")
    with open(output_path, "w") as f:
        f.write(c_array)
    print(f"Wrote {output_path}")


if __name__ == "__main__":
    main()
