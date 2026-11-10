/* DM3014 Interactive Devices, Ashley Hi 2026
   Week 11 – Remote Interaction
   This is the file you edit. 
   When someone walks up to the sensor, the page darkens, a light grows, and a line of text appears.
   When they leave, it fades back.
*/

// ── Settings ─────────────────────────────────────────────────────
const FAR_CM = 200; // treat anything further than this as "nobody"
const DAY = "#efe9dc"; // background when nobody is near
const NIGHT = "#12151c"; // background when someone is right there
const LAMP = [255, 208, 140];

// One of these appears each time someone arrives.
const LINES = [
  "You're in my no-no square",
  "Here comes the sun! dududududu",
  "Social Distance from me, please.",
  "You come into MY home uninvited and tell how I should arrange MY furniture?",
];

let smooth = FAR_CM; // eased distance, so the page moves gently
let shown = 0; // how visible the text is, 0 to 1
let line = "";

// ── Setup ────────────────────────────────────────────────────────

function setup() {
  createCanvas(windowWidth, windowHeight);
  textAlign(CENTER, CENTER);

  sensor.connect(firebaseConfig, DATA_PATH);

  sensor.on("near", () => {
    line = random(LINES);
  });

  sensor.on("far", () => {
    console.log("they left");
  });
}

// ── Draw ─────────────────────────────────────────────────────────

function draw() {
  // 1. Ease towards the latest reading. Readings arrive twice a
  //    second; easing fills in the gaps.
  const target = sensor.distance > 0 ? min(sensor.distance, FAR_CM) : FAR_CM;
  smooth = lerp(smooth, target, 0.08);

  // 2. Closeness: 0 when far away, 1 when right at the sensor.
  const closeness = constrain(map(smooth, FAR_CM, 10, 0, 1), 0, 1);

  // 3. The room darkens as they come closer.
  background(lerpColor(color(DAY), color(NIGHT), closeness));

  // 4. A light that grows with closeness.
  noStroke();
  for (let i = 6; i > 0; i--) {
    fill(LAMP[0], LAMP[1], LAMP[2], 40 * closeness);
    circle(width / 2, height / 2, i * 90 * closeness);
  }

  // 5. The line fades in while they're near.
  shown = lerp(shown, sensor.near ? 1 : 0, 0.06);
  fill(255, 255 * shown, 120 * shown);
  textFont("Karla");
  textSize(min(width, height) * 0.05);
  text(line, width / 2, height / 2 + min(width, height) * 0.28);

  // 6. The raw reading, small, bottom left.
  fill(closeness > 0.5 ? 200 : 60);
  textFont("JetBrains Mono");
  textSize(12);
  textAlign(LEFT, BOTTOM);
  text(
    sensor.distance < 0 ? "— cm" : nf(sensor.distance, 1, 1) + " cm",
    16,
    height - 16
  );
  textAlign(CENTER, CENTER);
}

function windowResized() {
  resizeCanvas(windowWidth, windowHeight);
}
