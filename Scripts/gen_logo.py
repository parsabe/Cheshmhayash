#!/usr/bin/env python3
"""
Generates the official Cheshm Hayash (His Eyes) game & installer logo/icons:
- Build/Windows/Application.ico (multi-resolution 256/128/64/48/32/16 Windows icon embedded into Cheshmhayash.exe)
- Cheshmhayash.png (512x512 project logo)
- Build/Windows/Logo.png (512x512)
- Packaged/Logo.png & Packaged/Application.ico
- Content/Splash/Splash.bmp & Content/Splash/EdSplash.bmp
"""

import math
import os
import struct
import zlib

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))


def clamp01(v: float) -> float:
    return 0.0 if v < 0.0 else (1.0 if v > 1.0 else v)


def smoothstep(edge0: float, edge1: float, x: float) -> float:
    if edge0 == edge1:
        return 1.0 if x >= edge1 else 0.0
    t = clamp01((x - edge0) / (edge1 - edge0))
    return t * t * (3.0 - 2.0 * t)


def lerp(a: float, b: float, t: float) -> float:
    return a + (b - a) * t


def lerp_color(c0, c1, t: float):
    t = clamp01(t)
    return (
        lerp(c0[0], c1[0], t),
        lerp(c0[1], c1[1], t),
        lerp(c0[2], c1[2], t),
    )


def sample_logo(nx: float, ny: float):
    """
    nx, ny in [-1, 1], where (0,0) is center, +y is down.
    Emblem design:
    - Dark obsidian circular medallion with a warm brushed-gold outer ring
    - Stylized almond eye contour ("Cheshm Hayash / His Eyes")
    - Inside the eye: glowing amber-crimson dawn sky, rising sun iris,
      snow-capped Mount Damavand silhouette at the bottom of the eye,
      and a warm golden highlight reflection.
    """
    r = math.hypot(nx, ny)
    if r > 0.985:
        return (0, 0, 0, 0)

    # Soft anti-aliased outer medallion edge
    outer_alpha = 1.0 - smoothstep(0.955, 0.985, r)

    # Background obsidian gradient
    bg = lerp_color((0.045, 0.048, 0.070), (0.015, 0.016, 0.024), clamp01(r))

    # Subtle radial warm glow behind the eye
    back_glow = math.exp(-((nx * 1.1) ** 2 + (ny * 1.4) ** 2) * 2.8)
    bg = lerp_color(bg, (0.22, 0.08, 0.05), back_glow * 0.55)

    # Outer gold medallion ring (r in [0.885, 0.945])
    ring_outer = smoothstep(0.875, 0.895, r) * (1.0 - smoothstep(0.935, 0.955, r))
    gold_shade = 0.68 + 0.32 * clamp01((-nx * 0.6 - ny * 0.8 + 1.0) * 0.5)
    gold_col = (0.88 * gold_shade, 0.67 * gold_shade, 0.32 * gold_shade)

    # Eye almond geometry: upper and lower lids
    # Width spans x in [-0.78, 0.78]
    ax = abs(nx) / 0.78
    if ax < 1.0:
        lid_half_h = 0.42 * (1.0 - ax * ax) ** 0.75
    else:
        lid_half_h = 0.0

    eye_dist = abs(ny) - lid_half_h
    inside_eye = 1.0 - smoothstep(-0.015, 0.015, eye_dist)

    col = bg

    if inside_eye > 0.0:
        # Sky gradient inside the eye (crimson-amber dawn from bottom to top)
        sky_t = clamp01((ny + 0.40) / 0.80)
        sky_top = (0.14, 0.09, 0.18)
        sky_mid = (0.68, 0.22, 0.12)
        sky_bot = (0.98, 0.66, 0.24)
        if sky_t < 0.55:
            eye_col = lerp_color(sky_top, sky_mid, sky_t / 0.55)
        else:
            eye_col = lerp_color(sky_mid, sky_bot, (sky_t - 0.55) / 0.45)

        # Rising sun / Iris in the center (nx=0, ny=-0.02)
        sun_dx = nx
        sun_dy = ny + 0.02
        sun_r = math.hypot(sun_dx, sun_dy)

        # Sun glow halo
        halo = math.exp(-(sun_r * sun_r) / 0.085)
        eye_col = lerp_color(eye_col, (1.0, 0.78, 0.36), halo * 0.75)

        # Sun disc (r = 0.25)
        sun_disc = 1.0 - smoothstep(0.235, 0.255, sun_r)
        sun_grad = lerp_color((1.0, 0.95, 0.75), (0.96, 0.48, 0.14), clamp01(sun_r / 0.25))
        eye_col = lerp_color(eye_col, sun_grad, sun_disc)

        # Dark pupil / inner eye core with a warm rim
        pupil_r = 0.105
        pupil_rim = smoothstep(0.095, 0.112, sun_r) * (1.0 - smoothstep(0.112, 0.130, sun_r))
        eye_col = lerp_color(eye_col, (0.45, 0.12, 0.06), pupil_rim * 0.7)
        pupil_mask = 1.0 - smoothstep(pupil_r - 0.012, pupil_r + 0.012, sun_r)
        eye_col = lerp_color(eye_col, (0.04, 0.03, 0.05), pupil_mask)

        # Specular catchlight in the eye (upper-left of pupil)
        catch_r = math.hypot(nx + 0.055, ny + 0.075)
        catch_mask = 1.0 - smoothstep(0.022, 0.034, catch_r)
        eye_col = lerp_color(eye_col, (1.0, 0.98, 0.92), catch_mask * 0.95)

        # Mount Damavand & Alborz range silhouette across the lower part of the eye
        # Peak centered slightly right (nx = 0.04), rising to ny = 0.05
        peak_x = nx - 0.03
        damavand_h = 0.04 + 0.75 * abs(peak_x) + 0.25 * (peak_x ** 2)
        ridge_left = 0.16 + 0.45 * abs(nx + 0.34)
        ridge_right = 0.15 + 0.48 * abs(nx - 0.38)
        mtn_y = min(damavand_h, ridge_left, ridge_right)

        if ny > mtn_y - 0.015:
            mtn_mask = smoothstep(mtn_y - 0.012, mtn_y + 0.012, ny)
            # Snow cap near Damavand summit
            snow_zone = (1.0 - smoothstep(0.04, 0.14, ny)) * (1.0 - smoothstep(0.09, 0.16, abs(peak_x)))
            mtn_col = lerp_color((0.07, 0.06, 0.09), (0.94, 0.88, 0.78), snow_zone * 0.85)
            eye_col = lerp_color(eye_col, mtn_col, mtn_mask)

        # Subtle vignette near the eyelids
        lid_vignette = smoothstep(lid_half_h * 0.65, lid_half_h, abs(ny))
        eye_col = lerp_color(eye_col, (0.05, 0.03, 0.04), lid_vignette * 0.55)

        col = lerp_color(col, eye_col, inside_eye)

    # Golden-amber eyeliner / lid contour
    if ax < 1.03:
        lid_stroke = (1.0 - smoothstep(0.0, 0.032, abs(eye_dist))) * (1.0 - smoothstep(0.96, 1.03, ax))
        col = lerp_color(col, (0.95, 0.72, 0.36), lid_stroke * 0.92)

    # Crimson tear / ember drop below the left-center of the eye
    tear_cx = -0.18
    tear_cy = 0.58
    if abs(ny - tear_cy) < 0.14 and abs(nx - tear_cx) < 0.08:
        ty = (ny - (tear_cy - 0.10)) / 0.20  # 0 at top tip, 1 at bottom bulb
        if 0.0 <= ty <= 1.0:
            half_w = 0.048 * (ty ** 0.85) * ((1.0 - ty * 0.35))
            td = abs(nx - tear_cx) - half_w
            if ty > 0.78:
                td = math.hypot(nx - tear_cx, ny - (tear_cy + 0.055)) - 0.038
            tear_mask = 1.0 - smoothstep(-0.008, 0.008, td)
            tear_col = lerp_color((0.88, 0.14, 0.10), (1.0, 0.48, 0.22), 1.0 - ty)
            col = lerp_color(col, tear_col, tear_mask)

    col = lerp_color(col, gold_col, ring_outer)

    return (
        int(clamp01(col[0]) * 255.0 + 0.5),
        int(clamp01(col[1]) * 255.0 + 0.5),
        int(clamp01(col[2]) * 255.0 + 0.5),
        int(clamp01(outer_alpha) * 255.0 + 0.5),
    )


def render_rgba(size: int) -> bytes:
    buf = bytearray(size * size * 4)
    inv = 2.0 / size
    idx = 0
    for y in range(size):
        ny = (y + 0.5) * inv - 1.0
        for x in range(size):
            nx = (x + 0.5) * inv - 1.0
            r, g, b, a = sample_logo(nx, ny)
            buf[idx] = r
            buf[idx + 1] = g
            buf[idx + 2] = b
            buf[idx + 3] = a
            idx += 4
    return bytes(buf)


def downsample_2x(src: bytes, src_size: int) -> bytes:
    dst_size = src_size // 2
    dst = bytearray(dst_size * dst_size * 4)
    stride = src_size * 4
    d_idx = 0
    for y in range(dst_size):
        r0 = (2 * y) * stride
        r1 = (2 * y + 1) * stride
        for x in range(dst_size):
            c0 = 2 * x * 4
            c1 = (2 * x + 1) * 4
            for ch in range(4):
                val = (
                    src[r0 + c0 + ch]
                    + src[r0 + c1 + ch]
                    + src[r1 + c0 + ch]
                    + src[r1 + c1 + ch]
                ) // 4
                dst[d_idx + ch] = val
            d_idx += 4
    return bytes(dst)


def encode_png(rgba: bytes, width: int, height: int) -> bytes:
    raw = bytearray()
    stride = width * 4
    for y in range(height):
        raw.append(0)  # filter type 0 (None)
        raw.extend(rgba[y * stride : (y + 1) * stride])

    def chunk(tag: bytes, data: bytes) -> bytes:
        crc = zlib.crc32(tag)
        crc = zlib.crc32(data, crc) & 0xFFFFFFFF
        return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", crc)

    ihdr = struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)
    idat = zlib.compress(bytes(raw), 9)
    return b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", ihdr) + chunk(b"IDAT", idat) + chunk(b"IEND", b"")


def write_ico(path: str, images_by_size: dict):
    sizes = sorted(images_by_size.keys(), reverse=True)
    header = struct.pack("<HHH", 0, 1, len(sizes))
    entries = bytearray()
    payloads = bytearray()
    offset = 6 + 16 * len(sizes)

    for s in sizes:
        png_bytes = encode_png(images_by_size[s], s, s)
        w_byte = 0 if s >= 256 else s
        h_byte = 0 if s >= 256 else s
        entries.extend(
            struct.pack(
                "<BBBBHHII",
                w_byte,
                h_byte,
                0,
                0,
                1,
                32,
                len(png_bytes),
                offset,
            )
        )
        payloads.extend(png_bytes)
        offset += len(png_bytes)

    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "wb") as f:
        f.write(header + entries + payloads)


def write_splash_bmp(path: str, logo_rgba: bytes, logo_size: int, width: int = 600, height: int = 200):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    row_bytes = width * 3
    pad = (4 - (row_bytes % 4)) % 4
    stride = row_bytes + pad
    pixel_data = bytearray(stride * height)

    # Place the logo on the left-center of the dark splash banner
    logo_target = 156
    ox = (width - logo_target) // 2
    oy = (height - logo_target) // 2

    for y in range(height):
        bmp_y = height - 1 - y
        row_start = bmp_y * stride
        for x in range(width):
            # Subtle dark crimson-obsidian background
            bg_r, bg_g, bg_b = 10, 10, 15
            if ox <= x < ox + logo_target and oy <= y < oy + logo_target:
                lx = int((x - ox) * logo_size / logo_target)
                ly = int((y - oy) * logo_size / logo_target)
                li = (ly * logo_size + lx) * 4
                lr, lg, lb, la = logo_rgba[li], logo_rgba[li + 1], logo_rgba[li + 2], logo_rgba[li + 3]
                alpha = la / 255.0
                bg_r = int(bg_r * (1.0 - alpha) + lr * alpha)
                bg_g = int(bg_g * (1.0 - alpha) + lg * alpha)
                bg_b = int(bg_b * (1.0 - alpha) + lb * alpha)
            p = row_start + x * 3
            pixel_data[p] = bg_b
            pixel_data[p + 1] = bg_g
            pixel_data[p + 2] = bg_r

    file_size = 14 + 40 + len(pixel_data)
    bmp_header = struct.pack("<2sIHHI", b"BM", file_size, 0, 0, 54)
    dib_header = struct.pack(
        "<IIIHHIIIIII",
        40,
        width,
        height,
        1,
        24,
        0,
        len(pixel_data),
        2835,
        2835,
        0,
        0,
    )
    with open(path, "wb") as f:
        f.write(bmp_header + dib_header + pixel_data)


def main():
    print("Rendering Cheshm Hayash logo and installer icons...")
    img512 = render_rgba(512)
    img256 = downsample_2x(img512, 512)
    img128 = downsample_2x(img256, 256)
    img64 = downsample_2x(img128, 128)
    img32 = downsample_2x(img64, 64)
    img16 = downsample_2x(img32, 32)
    img48 = render_rgba(48)

    png512 = encode_png(img512, 512, 512)

    for png_path in [
        os.path.join(ROOT, "Cheshmhayash.png"),
        os.path.join(ROOT, "Build", "Windows", "Logo.png"),
        os.path.join(ROOT, "Packaged", "Logo.png"),
        os.path.join(ROOT, "Packaged", "Windows", "Logo.png"),
    ]:
        os.makedirs(os.path.dirname(png_path), exist_ok=True)
        with open(png_path, "wb") as f:
            f.write(png512)
        print("  wrote", png_path)

    ico_map = {256: img256, 128: img128, 64: img64, 48: img48, 32: img32, 16: img16}
    for ico_path in [
        os.path.join(ROOT, "Build", "Windows", "Application.ico"),
        os.path.join(ROOT, "Packaged", "Application.ico"),
        os.path.join(ROOT, "Packaged", "Windows", "Application.ico"),
    ]:
        write_ico(ico_path, ico_map)
        print("  wrote", ico_path)

    for bmp_path in [
        os.path.join(ROOT, "Content", "Splash", "Splash.bmp"),
        os.path.join(ROOT, "Content", "Splash", "EdSplash.bmp"),
    ]:
        write_splash_bmp(bmp_path, img256, 256)
        print("  wrote", bmp_path)

    print("Logo and installer icon generation complete.")


if __name__ == "__main__":
    main()
