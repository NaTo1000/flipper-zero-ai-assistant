/**
 * scene3d.js — Three.js 3D Flipper Zero device scene
 * Renders an animated Flipper Zero model with orange glow effects
 */

(function () {
  'use strict';

  // ── Scene constants ────────────────────────────────────────────────
  const PARTICLE_COUNT   = 220;
  const GPIO_PIN_COUNT   = 8;
  const GPIO_START_X     = -1.2;
  const GPIO_SPACING_X   = 0.16;
  const GPIO_Y           = 0.71;
  const GPIO_Z           = 0.05;

  const canvas = document.getElementById('canvas3d');
  if (!canvas) return;

  // ── Renderer ──────────────────────────────────────────────────────
  const renderer = new THREE.WebGLRenderer({ canvas, antialias: true, alpha: true });
  renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
  renderer.setSize(canvas.clientWidth, canvas.clientHeight);
  renderer.shadowMap.enabled = true;
  renderer.shadowMap.type = THREE.PCFSoftShadowMap;
  renderer.toneMapping = THREE.ACESFilmicToneMapping;
  renderer.toneMappingExposure = 1.2;

  // ── Scene & Camera ─────────────────────────────────────────────────
  const scene = new THREE.Scene();
  const camera = new THREE.PerspectiveCamera(
    45,
    canvas.clientWidth / canvas.clientHeight,
    0.1,
    100
  );
  camera.position.set(0, 0.5, 6);

  // ── Lights ─────────────────────────────────────────────────────────
  const ambientLight = new THREE.AmbientLight(0x111111, 1.5);
  scene.add(ambientLight);

  // Key light — orange
  const keyLight = new THREE.PointLight(0xFF6B00, 8, 20);
  keyLight.position.set(3, 4, 4);
  keyLight.castShadow = true;
  scene.add(keyLight);

  // Fill light — cool blue
  const fillLight = new THREE.PointLight(0x0088FF, 3, 20);
  fillLight.position.set(-4, 0, 3);
  scene.add(fillLight);

  // Rim light — deep orange
  const rimLight = new THREE.PointLight(0xFF4400, 4, 20);
  rimLight.position.set(0, -3, -4);
  scene.add(rimLight);

  // ── Materials ──────────────────────────────────────────────────────
  const bodyMat = new THREE.MeshStandardMaterial({
    color: 0x1A1A1A,
    roughness: 0.3,
    metalness: 0.7,
  });

  const orangeMat = new THREE.MeshStandardMaterial({
    color: 0xFF6B00,
    roughness: 0.25,
    metalness: 0.4,
    emissive: 0xFF3300,
    emissiveIntensity: 0.3,
  });

  const screenMat = new THREE.MeshStandardMaterial({
    color: 0x0D1A0D,
    roughness: 0.1,
    metalness: 0.05,
    emissive: 0x39FF14,
    emissiveIntensity: 0.25,
  });

  const buttonMat = new THREE.MeshStandardMaterial({
    color: 0x2A2A2A,
    roughness: 0.5,
    metalness: 0.4,
  });

  const btnActiveMat = new THREE.MeshStandardMaterial({
    color: 0xFF6B00,
    roughness: 0.3,
    metalness: 0.4,
    emissive: 0xFF3300,
    emissiveIntensity: 0.6,
  });

  const glassMat = new THREE.MeshStandardMaterial({
    color: 0x223322,
    roughness: 0.05,
    metalness: 0.1,
    transparent: true,
    opacity: 0.85,
    envMapIntensity: 1.5,
  });

  // ── Build Flipper Zero Body ─────────────────────────────────────────
  const flipperGroup = new THREE.Group();

  // Main body — rounded box approximation
  const bodyGeo = new THREE.BoxGeometry(3.0, 1.55, 0.55, 2, 2, 2);
  const body = new THREE.Mesh(bodyGeo, bodyMat);
  body.castShadow = true;
  flipperGroup.add(body);

  // Orange accent strip (top edge)
  const stripGeo = new THREE.BoxGeometry(3.0, 0.12, 0.57);
  const strip = new THREE.Mesh(stripGeo, orangeMat);
  strip.position.set(0, 0.835, 0);
  flipperGroup.add(strip);

  // Orange accent strip (bottom edge)
  const strip2 = strip.clone();
  strip2.position.set(0, -0.835, 0);
  flipperGroup.add(strip2);

  // Screen bezel
  const bezelGeo = new THREE.BoxGeometry(1.6, 0.85, 0.06);
  const bezel = new THREE.Mesh(bezelGeo, new THREE.MeshStandardMaterial({ color: 0x111111, roughness: 0.4, metalness: 0.5 }));
  bezel.position.set(-0.58, 0.08, 0.305);
  flipperGroup.add(bezel);

  // Screen glass
  const screenGeo = new THREE.BoxGeometry(1.4, 0.7, 0.02);
  const screen = new THREE.Mesh(screenGeo, glassMat);
  screen.position.set(-0.58, 0.08, 0.34);
  flipperGroup.add(screen);

  // Screen glow plane (behind glass)
  const glowGeo = new THREE.PlaneGeometry(1.38, 0.68);
  const glowMat = new THREE.MeshBasicMaterial({
    color: 0x39FF14,
    transparent: true,
    opacity: 0.18,
  });
  const glowPlane = new THREE.Mesh(glowGeo, glowMat);
  glowPlane.position.set(-0.58, 0.08, 0.325);
  flipperGroup.add(glowPlane);

  // ── D-pad ───────────────────────────────────────────────────────────
  const dpadGroup = new THREE.Group();
  dpadGroup.position.set(0.82, 0.1, 0.31);

  const dpadCenterGeo = new THREE.CylinderGeometry(0.14, 0.14, 0.06, 12);
  const dpadCenter = new THREE.Mesh(dpadCenterGeo, buttonMat);
  dpadCenter.rotation.x = Math.PI / 2;
  dpadGroup.add(dpadCenter);

  const dpadArmGeo = new THREE.BoxGeometry(0.12, 0.35, 0.06);
  const dirs = [[0, 0.22, 0], [0, -0.22, 0], [-0.22, 0, 0], [0.22, 0, 0]];
  dirs.forEach(([x, y, z]) => {
    const arm = new THREE.Mesh(dpadArmGeo, buttonMat);
    arm.position.set(x, y, z);
    if (x !== 0) arm.rotation.z = Math.PI / 2;
    dpadGroup.add(arm);
  });

  flipperGroup.add(dpadGroup);

  // ── Action Buttons (A / B) ──────────────────────────────────────────
  const btnGeo = new THREE.CylinderGeometry(0.1, 0.1, 0.07, 16);
  const btnPositions = [
    [1.18, 0.3, 0.31],  // A
    [1.38, 0.05, 0.31], // B
  ];
  const btnMats = [btnActiveMat, buttonMat];
  btnPositions.forEach(([x, y, z], i) => {
    const btn = new THREE.Mesh(btnGeo, btnMats[i]);
    btn.rotation.x = Math.PI / 2;
    btn.position.set(x, y, z);
    flipperGroup.add(btn);
  });

  // ── Back / OK buttons ──────────────────────────────────────────────
  const smallBtnGeo = new THREE.CylinderGeometry(0.065, 0.065, 0.065, 12);
  const smallBtns = [
    [0.75, -0.3, 0.31],
    [1.05, -0.3, 0.31],
    [1.35, -0.3, 0.31],
  ];
  smallBtns.forEach(([x, y, z]) => {
    const sb = new THREE.Mesh(smallBtnGeo, buttonMat);
    sb.rotation.x = Math.PI / 2;
    sb.position.set(x, y, z);
    flipperGroup.add(sb);
  });

  // ── IR Blaster (top right) ─────────────────────────────────────────
  const irGeo = new THREE.CylinderGeometry(0.045, 0.045, 0.12, 10);
  const irMat = new THREE.MeshStandardMaterial({ color: 0x1A0000, roughness: 0.4, metalness: 0.2 });
  const ir = new THREE.Mesh(irGeo, irMat);
  ir.position.set(1.35, 0.71, 0.1);
  flipperGroup.add(ir);

  // ── USB-C port (bottom) ────────────────────────────────────────────
  const usbGeo = new THREE.BoxGeometry(0.22, 0.1, 0.06);
  const usbMat = new THREE.MeshStandardMaterial({ color: 0x444444, roughness: 0.4, metalness: 0.8 });
  const usb = new THREE.Mesh(usbGeo, usbMat);
  usb.position.set(0, -0.835, 0.15);
  usb.rotation.x = Math.PI / 2;
  flipperGroup.add(usb);

  // ── SD Card slot (right side) ─────────────────────────────────────
  const sdGeo = new THREE.BoxGeometry(0.06, 0.18, 0.28);
  const sd = new THREE.Mesh(sdGeo, usbMat);
  sd.position.set(1.535, -0.45, 0.1);
  flipperGroup.add(sd);

  // ── GPIO pins (top strip) ─────────────────────────────────────────
  for (let i = 0; i < GPIO_PIN_COUNT; i++) {
    const pinGeo = new THREE.CylinderGeometry(0.018, 0.018, 0.1, 8);
    const pinMat = new THREE.MeshStandardMaterial({ color: 0xCCCC88, metalness: 0.9, roughness: 0.2 });
    const pin = new THREE.Mesh(pinGeo, pinMat);
    pin.position.set(GPIO_START_X + i * GPIO_SPACING_X, GPIO_Y, GPIO_Z);
    flipperGroup.add(pin);
  }

  // ── Orange LED ─────────────────────────────────────────────────────
  const ledGeo = new THREE.SphereGeometry(0.055, 12, 12);
  const ledMat = new THREE.MeshStandardMaterial({
    color: 0xFF6B00,
    emissive: 0xFF6B00,
    emissiveIntensity: 3,
    roughness: 0.1,
    metalness: 0.0,
  });
  const led = new THREE.Mesh(ledGeo, ledMat);
  led.position.set(-1.35, 0.55, 0.305);
  flipperGroup.add(led);

  // ── Point light at LED position ────────────────────────────────────
  const ledLight = new THREE.PointLight(0xFF6B00, 2, 3);
  ledLight.position.copy(led.position);
  flipperGroup.add(ledLight);

  // ── Add reflective floor plane ─────────────────────────────────────
  const floorGeo = new THREE.PlaneGeometry(20, 20);
  const floorMat = new THREE.MeshStandardMaterial({
    color: 0x111111,
    roughness: 0.8,
    metalness: 0.4,
  });
  const floor = new THREE.Mesh(floorGeo, floorMat);
  floor.rotation.x = -Math.PI / 2;
  floor.position.y = -1.8;
  floor.receiveShadow = true;
  scene.add(floor);

  flipperGroup.castShadow = true;
  scene.add(flipperGroup);

  // ── Floating particles ─────────────────────────────────────────────
  const positions = new Float32Array(PARTICLE_COUNT * 3);
  for (let i = 0; i < PARTICLE_COUNT; i++) {
    positions[i * 3]     = (Math.random() - 0.5) * 14;
    positions[i * 3 + 1] = (Math.random() - 0.5) * 8;
    positions[i * 3 + 2] = (Math.random() - 0.5) * 8 - 2;
  }
  const partGeo = new THREE.BufferGeometry();
  partGeo.setAttribute('position', new THREE.BufferAttribute(positions, 3));
  const partMat = new THREE.PointsMaterial({
    color: 0xFF6B00,
    size: 0.03,
    transparent: true,
    opacity: 0.55,
  });
  const particles = new THREE.Points(partGeo, partMat);
  scene.add(particles);

  // ── Mouse interactivity ────────────────────────────────────────────
  let mouseX = 0, mouseY = 0;
  document.addEventListener('mousemove', (e) => {
    mouseX = (e.clientX / window.innerWidth - 0.5) * 2;
    mouseY = (e.clientY / window.innerHeight - 0.5) * 2;
  });

  // ── Resize handler ─────────────────────────────────────────────────
  function onResize() {
    const w = canvas.clientWidth;
    const h = canvas.clientHeight;
    camera.aspect = w / h;
    camera.updateProjectionMatrix();
    renderer.setSize(w, h);
  }
  window.addEventListener('resize', onResize);

  // ── Animation loop ─────────────────────────────────────────────────
  let t = 0;
  function animate() {
    requestAnimationFrame(animate);
    t += 0.012;

    // Gentle auto-rotate + mouse parallax
    flipperGroup.rotation.y = Math.sin(t * 0.3) * 0.4 + mouseX * 0.25;
    flipperGroup.rotation.x = Math.sin(t * 0.2) * 0.12 - mouseY * 0.12;
    flipperGroup.position.y = Math.sin(t * 0.5) * 0.12;

    // Pulse screen glow
    glowMat.opacity = 0.12 + Math.sin(t * 1.5) * 0.06;

    // Pulse LED
    ledMat.emissiveIntensity = 2 + Math.sin(t * 2) * 1;
    ledLight.intensity = 1.5 + Math.sin(t * 2) * 0.8;

    // Drift particles
    const pos = particles.geometry.attributes.position;
    for (let i = 0; i < PARTICLE_COUNT; i++) {
      pos.array[i * 3 + 1] += 0.003;
      if (pos.array[i * 3 + 1] > 4) pos.array[i * 3 + 1] = -4;
    }
    pos.needsUpdate = true;
    particles.rotation.y += 0.0005;

    // Orbit key light
    keyLight.position.x = Math.cos(t * 0.4) * 5;
    keyLight.position.z = Math.sin(t * 0.4) * 4 + 2;

    renderer.render(scene, camera);
  }

  animate();
})();
