/* Cheshm Hayash: promotional site.
   Plain JS: IntersectionObserver for reveals, one requestAnimationFrame loop for scroll effects.
   No libraries, no tracking, no cookies, no storage. */
(function () {
  "use strict";

  var root = document.documentElement;
  var reduced = window.matchMedia && window.matchMedia("(prefers-reduced-motion: reduce)").matches;
  var params = new URLSearchParams(location.search);

  function $(sel, ctx) { return (ctx || document).querySelector(sel); }
  function $$(sel, ctx) { return Array.prototype.slice.call((ctx || document).querySelectorAll(sel)); }
  function clamp(v, a, b) { return Math.min(b === undefined ? 1 : b, Math.max(a === undefined ? 0 : a, v)); }
  function range(v, a, b) { return clamp((v - a) / (b - a)); }
  function smooth(t) { return t * t * (3 - 2 * t); }
  function lerp(a, b, t) { return a + (b - a) * t; }

  /* ---------------- language ---------------- */
  var toggle = $(".lang-toggle");
  function setLang(lang) {
    root.setAttribute("data-lang", lang);
    root.lang = lang;
    root.dir = lang === "fa" ? "rtl" : "ltr";
    toggle.setAttribute("aria-label", lang === "fa" ? "Switch to English" : "تغییر زبان به فارسی");
    document.title = lang === "fa" ? "چشم‌هایش · Cheshm Hayash" : "Cheshm Hayash · چشم‌هایش";
  }
  var browserFa = (navigator.languages || [navigator.language || ""]).some(function (l) { return /^fa\b/i.test(l); });
  var startLang = params.get("lang");
  setLang(startLang === "fa" || startLang === "en" ? startLang : (browserFa ? "fa" : "en"));
  toggle.addEventListener("click", function () {
    setLang(root.getAttribute("data-lang") === "fa" ? "en" : "fa");
  });

  /* ---------------- REC timestamp ---------------- */
  var rec = $(".rec");
  var recTime = $(".rec-time");
  var t0 = Date.now();
  function pad(n) { return (n < 10 ? "0" : "") + n; }
  setInterval(function () {
    var s = Math.floor((Date.now() - t0) / 1000);
    recTime.textContent = pad(Math.floor(s / 3600)) + ":" + pad(Math.floor(s / 60) % 60) + ":" + pad(s % 60);
  }, 1000);

  /* ---------------- hero title, letter by letter ---------------- */
  var title = $(".title");
  if (title && !reduced) {
    var text = title.textContent.trim();
    var letters = Array.from(text);
    title.setAttribute("aria-label", text);
    title.textContent = "";
    letters.forEach(function (ch, i) {
      var span = document.createElement("span");
      span.textContent = ch;
      span.setAttribute("aria-hidden", "true");
      span.style.setProperty("--i", i);
      title.appendChild(span);
    });
    // afterwards, put the word back whole so the Persian letters always join correctly
    setTimeout(function () { title.textContent = text; }, 900 + letters.length * 170 + 1500);
  }

  /* ---------------- iris fibres (drawn once) ---------------- */
  var fibres = $(".iris-fibres");
  if (fibres) {
    var d = "";
    for (var i = 0; i < 72; i++) {
      var a = (i / 72) * Math.PI * 2 + (i % 3) * 0.02;
      var r1 = 74 + (i % 4) * 6, r2 = 150 + (i % 5) * 7;
      d += "M" + (500 + Math.cos(a) * r1).toFixed(1) + " " + (260 + Math.sin(a) * r1).toFixed(1) +
           "L" + (500 + Math.cos(a) * r2).toFixed(1) + " " + (260 + Math.sin(a) * r2).toFixed(1);
    }
    var path = document.createElementNS("http://www.w3.org/2000/svg", "path");
    path.setAttribute("d", d);
    fibres.appendChild(path);
  }

  /* ---------------- Tehran skyline, built like the game builds it ---------------- */
  function rng(seed) {
    return function () {
      seed = (seed * 1664525 + 1013904223) >>> 0;
      return seed / 4294967296;
    };
  }
  function skyline(seed, w, h, minH, maxH) {
    var r = rng(seed), x = 0, d = "M0 " + h;
    while (x < w) {
      var bw = 40 + r() * 90;
      var bh = minH + r() * (maxH - minH);
      var top = h - bh;
      d += " L" + x.toFixed(0) + " " + top.toFixed(0);
      // a water tank or a satellite dish on some roofs
      if (r() < 0.45) {
        var tx = x + 8 + r() * (bw - 30);
        d += " L" + tx.toFixed(0) + " " + top.toFixed(0) + " L" + tx.toFixed(0) + " " + (top - 14).toFixed(0) +
             " L" + (tx + 16).toFixed(0) + " " + (top - 14).toFixed(0) + " L" + (tx + 16).toFixed(0) + " " + top.toFixed(0);
      } else if (r() < 0.5) {
        var dx = x + 10 + r() * (bw - 26);
        d += " L" + dx.toFixed(0) + " " + top.toFixed(0) + " L" + (dx + 3).toFixed(0) + " " + (top - 10).toFixed(0) +
             " L" + (dx + 12).toFixed(0) + " " + (top - 12).toFixed(0) + " L" + (dx + 9).toFixed(0) + " " + top.toFixed(0);
      }
      x += bw;
      d += " L" + x.toFixed(0) + " " + top.toFixed(0);
      if (r() < 0.25) { x += 6 + r() * 14; d += " L" + x.toFixed(0) + " " + (h - minH * 0.5).toFixed(0); }
    }
    d += " L" + w + " " + h + " Z";
    return '<svg viewBox="0 0 ' + w + " " + h + '" preserveAspectRatio="none"><path d="' + d + '"/></svg>';
  }
  var storySky = $(".layer-skyline");
  if (storySky) {
    storySky.innerHTML = skyline(1401, 1600, 320, 90, 250);
    var p = storySky.querySelector("path");
    p.setAttribute("fill", "#120706");
    // a few lit windows
    var r = rng(24), win = "";
    for (var k = 0; k < 60; k++) {
      win += '<rect x="' + (r() * 1600).toFixed(0) + '" y="' + (150 + r() * 160).toFixed(0) + '" width="7" height="9" fill="' + (r() < 0.8 ? "#ffb066" : "#9fc0ff") + '" opacity="' + (0.25 + r() * 0.5).toFixed(2) + '"/>';
    }
    storySky.querySelector("svg").insertAdjacentHTML("beforeend", win);
  }
  var sunSky = $(".sun-skyline");
  if (sunSky) { sunSky.innerHTML = skyline(1404, 1600, 260, 60, 220); }

  /* ---------------- reveals ---------------- */
  function reveal(sel, options) {
    var els = $$(sel);
    if (reduced || !("IntersectionObserver" in window)) {
      els.forEach(function (el) { el.classList.add("is-in"); });
      return;
    }
    var io = new IntersectionObserver(function (entries) {
      entries.forEach(function (e) {
        if (e.isIntersecting) { e.target.classList.add("is-in"); io.unobserve(e.target); }
      });
    }, options);
    els.forEach(function (el) { io.observe(el); });
  }
  reveal(".reveal", { threshold: 0.18, rootMargin: "0px 0px -6% 0px" });
  // the poem: each line only when it reaches the middle of the screen, so they come one by one
  reveal(".poem-line", { threshold: 0, rootMargin: "-38% 0px -38% 0px" });
  reveal(".dedication", { threshold: 0.6 });

  // the REC dot stops at the dedication (as it is gone in the game's last chapter)
  var dedication = $(".dedication");
  if ("IntersectionObserver" in window && dedication) {
    new IntersectionObserver(function (entries) {
      rec.classList.toggle("is-off", entries[0].isIntersecting);
    }, { threshold: 0.35 }).observe(dedication);
  }

  /* ---------------- trailer (YouTube loads only after a click) ---------------- */
  var trailer = $(".trailer");
  var playBtn = $(".trailer-play");
  if (trailer && playBtn) {
    playBtn.addEventListener("click", function () {
      var id = (trailer.getAttribute("data-youtube") || "").trim();
      if (!id || id === "YOUTUBE_VIDEO_ID") { trailer.classList.add("soon"); return; }
      var f = document.createElement("iframe");
      f.src = "https://www.youtube-nocookie.com/embed/" + encodeURIComponent(id) + "?autoplay=1&rel=0&modestbranding=1";
      f.title = "Cheshm Hayash: trailer";
      f.allow = "autoplay; encrypted-media; picture-in-picture; fullscreen";
      f.allowFullscreen = true;
      f.referrerPolicy = "strict-origin-when-cross-origin";
      trailer.replaceChildren(f);
    });
  }

  /* ---------------- itch.io button: until the real link is in, say "coming soon" ---------------- */
  var playLink = $(".play-button");
  if (playLink && /YOUR-NAME/.test(playLink.getAttribute("href") || "")) {
    playLink.removeAttribute("href");
    playLink.classList.add("soon");
    playLink.innerHTML = '<span class="t-fa">به‌زودی در <bdi>itch.io</bdi></span><span class="t-en">Coming soon to itch.io</span>';
  }

  /* ---------------- scroll-driven scenes ---------------- */
  var vw = window.innerWidth, vh = window.innerHeight;
  var hero = $("#hero"), heroBg = $(".hero-bg"), eye = $(".eye"), heroTitle = $(".hero-title"),
      heroBlack = $(".hero-black"), cue = $(".scroll-cue");
  var sunrise = $("#sunrise"), sky = $(".sky"), sun = $(".sun"), sunPhoto = $(".sun-photo"),
      sunText = $(".sun-text"), sunMountain = $(".sun-mountain"), sunSkyline = $(".sun-skyline");
  var layers = $$("[data-speed]");
  var eyeScale = 1;

  function progress(section) {
    var r = section.getBoundingClientRect();
    var total = r.height - vh;
    return total > 0 ? clamp(-r.top / total) : (r.top < 0 ? 1 : 0);
  }

  function heroFrame() {
    var p = progress(hero);
    // zoom into the pupil: scale until the black pupil covers the whole screen
    // scaling keeps the centre where it is, so the box centre is the pupil
    var er = eye.getBoundingClientRect();
    var w = er.width / eyeScale || 1;      // (an <svg> has no offsetWidth)
    var pupil = (66 / 1000) * w;
    var cx = er.left + er.width / 2, cy = er.top + er.height / 2;
    var far = Math.max(Math.hypot(cx, cy), Math.hypot(vw - cx, cy), Math.hypot(cx, vh - cy), Math.hypot(vw - cx, vh - cy));
    var need = (far / pupil) * 1.08;
    var t = range(p, 0.06, 0.82);
    var s = 1 + (need - 1) * t * t * t;
    eyeScale = s;
    eye.style.transform = "scale(" + s.toFixed(4) + ")";
    eye.classList.toggle("zooming", s > 1.3);
    heroTitle.style.opacity = (1 - range(p, 0, 0.2)).toFixed(3);
    heroTitle.style.transform = "translateY(" + (-80 * range(p, 0, 0.3)).toFixed(1) + "px)";
    cue.style.opacity = (1 - range(p, 0, 0.06)).toFixed(3);
    // the street behind dissolves while we fall into the eye
    heroBg.style.opacity = (1 - range(p, 0.25, 0.7)).toFixed(3);
    heroBg.style.transform = "scale(" + (1 + 0.35 * smooth(range(p, 0, 0.8))).toFixed(4) + ")";
    heroBlack.style.opacity = range(p, 0.78, 0.94).toFixed(3);
  }

  var NIGHT = { top: [8, 4, 10], mid: [42, 8, 6], low: [70, 14, 8] };
  var DAWN = { top: [126, 150, 196], mid: [255, 178, 102], low: [255, 226, 168] };
  function mix(a, b, t) {
    return "rgb(" + Math.round(lerp(a[0], b[0], t)) + "," + Math.round(lerp(a[1], b[1], t)) + "," + Math.round(lerp(a[2], b[2], t)) + ")";
  }
  function sunriseFrame(p) {
    var t = smooth(range(p, 0.05, 0.75));
    sky.style.background = "linear-gradient(180deg," + mix(NIGHT.top, DAWN.top, t) + " 0%," + mix(NIGHT.mid, DAWN.mid, t) + " 58%," + mix(NIGHT.low, DAWN.low, t) + " 82%)";
    // the sun climbs from below the mountains
    var rise = smooth(range(p, 0.02, 0.7));
    sun.style.transform = "translateY(" + ((1 - rise) * 55).toFixed(2) + "vh) scale(" + (0.8 + 0.3 * rise).toFixed(3) + ")";
    sun.style.opacity = (0.3 + 0.7 * rise).toFixed(3);
    // silhouettes warm from black-red to deep amber
    var m = smooth(range(p, 0.2, 0.8));
    sunMountain.style.fill = mix([26, 11, 10], [120, 58, 34], m);
    var path = sunSkyline.querySelector("path");
    if (path) { path.style.fill = mix([13, 6, 6], [74, 34, 20], m); }
    // finally the real in-game sunrise
    var ph = smooth(range(p, 0.6, 0.92));
    sunPhoto.style.opacity = ph.toFixed(3);
    sunPhoto.style.transform = "scale(" + (1.12 - 0.12 * ph).toFixed(4) + ")";
    // the drawn mountain, city and sun give way to the photo
    var drawn = (1 - ph).toFixed(3);
    sunMountain.style.opacity = drawn;
    sunSkyline.style.opacity = drawn;
    sun.style.opacity = ((0.3 + 0.7 * rise) * (1 - ph)).toFixed(3);
    var tx = smooth(range(p, 0.3, 0.55)) * (1 - range(p, 0.93, 1));
    sunText.style.opacity = tx.toFixed(3);
    sunText.style.transform = "translateY(" + ((1 - smooth(range(p, 0.3, 0.55))) * 30).toFixed(1) + "px)";
  }

  function parallaxFrame() {
    for (var i = 0; i < layers.length; i++) {
      var el = layers[i];
      var box = el.parentElement.getBoundingClientRect();
      if (box.bottom < -200 || box.top > vh + 200) continue;
      var speed = parseFloat(el.getAttribute("data-speed")) || 0;
      var offset = (box.top + box.height / 2 - vh / 2) * speed;
      el.style.transform = "translate3d(0," + offset.toFixed(1) + "px,0)";
    }
  }

  var queued = false;
  function frame() {
    queued = false;
    heroFrame();
    sunriseFrame(progress(sunrise));
    parallaxFrame();
  }
  function request() {
    if (!queued) { queued = true; window.requestAnimationFrame(frame); }
  }

  if (reduced) {
    // no movement: a still eye, and the sunrise already up
    sunriseFrame(1);
    sunText.style.opacity = "1";
  } else {
    window.addEventListener("scroll", request, { passive: true });
    window.addEventListener("resize", function () { vw = window.innerWidth; vh = window.innerHeight; request(); });
    frame();
  }

  // ?at=sectionId&f=0.5 opens the page at that point (handy for screenshots)
  var at = params.get("at");
  if (at && document.getElementById(at)) {
    var el = document.getElementById(at);
    var f = parseFloat(params.get("f") || "0") || 0;
    window.scrollTo(0, el.offsetTop + f * Math.max(0, el.offsetHeight - vh));
    if (!reduced) { frame(); }
  }
})();
