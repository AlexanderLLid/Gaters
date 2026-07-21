import argparse
import json
from pathlib import Path


def _rounded(vertices):
    return [[round(float(value), 5) for value in vertex] for vertex in vertices]


def _triangles(faces):
    result = []
    for face in faces:
        for index in range(1, len(face) - 1):
            result.extend((face[0], face[index], face[index + 1]))
    return result


def render(readback, output):
    modules = readback["modules"]
    groups = []
    for module in modules:
        groups.append(3 if module == "head" else 0 if module == "torso" else 1 if "arm" in module else 2)
    peak = max(
        readback["frames"],
        key=lambda frame: max(
            sum((actual[axis] - rest[axis]) ** 2 for axis in range(3))
            for rest, actual in zip(readback["positions"], frame["positions"])
        ),
    )
    data = {
        "vertexCount": len(readback["positions"]),
        "faceCount": len(readback["faces"]),
        "rest": _rounded(readback["positions"]),
        "peak": _rounded(peak["positions"]),
        "triangles": _triangles(readback["faces"]),
        "groups": groups,
    }
    payload = json.dumps(data, separators=(",", ":"), allow_nan=False)
    fragment = f'''<div id="gaters-model-inspector">
  <div class="viz-controls" aria-label="Model inspection controls">
    <button type="button" class="btn btn-primary" data-pose="rest" aria-pressed="true">Rest pose</button>
    <button type="button" class="btn" data-pose="peak" aria-pressed="false">Peak pose</button>
    <label class="form-check form-switch">
      <input class="form-check-input" type="checkbox" id="gaters-wireframe">
      <span class="form-check-label">Wireframe</span>
    </label>
    <button type="button" class="btn btn-ghost" data-reset>Reset view</button>
  </div>
  <div class="card">
    <div class="model-viewport" role="img" aria-label="Rotatable generated humanoid mesh in rest or peak pose"></div>
  </div>
  <div class="text-small text-muted model-status">13,149 vertices · 13,822 faces · drag to rotate · wheel to zoom</div>
</div>
<style>
  #gaters-model-inspector {{ width: 100%; }}
  #gaters-model-inspector .viz-controls {{ margin-bottom: 0.75rem; }}
  #gaters-model-inspector .model-viewport {{ width: 100%; height: 560px; }}
  #gaters-model-inspector .model-status {{ margin-top: 0.5rem; }}
  @media (max-width: 520px) {{ #gaters-model-inspector .model-viewport {{ height: 420px; }} }}
</style>
<script type="module">
  import * as THREE from 'https://esm.sh/three@0.179.1';
  import {{ OrbitControls }} from 'https://esm.sh/three@0.179.1/examples/jsm/controls/OrbitControls.js';

  const root = document.getElementById('gaters-model-inspector');
  const viewport = root.querySelector('.model-viewport');
  const data = {payload};
  const scene = new THREE.Scene();
  const camera = new THREE.PerspectiveCamera(35, 1, 0.01, 100);
  const renderer = new THREE.WebGLRenderer({{ antialias: true, alpha: true }});
  renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
  renderer.outputColorSpace = THREE.SRGBColorSpace;
  viewport.appendChild(renderer.domElement);

  const resolveColor = (token) => {{
    const probe = document.createElement('span');
    probe.style.color = `var(${{token}})`;
    probe.style.display = 'none';
    root.appendChild(probe);
    const color = getComputedStyle(probe).color;
    probe.remove();
    return new THREE.Color(color);
  }};
  const palette = ['--viz-series-1', '--viz-series-2', '--viz-series-3', '--viz-series-4'].map(resolveColor);
  const positions = new Float32Array(data.vertexCount * 3);
  const colors = new Float32Array(data.vertexCount * 3);
  for (let i = 0; i < data.vertexCount; i++) {{
    const color = palette[data.groups[i]];
    colors.set([color.r, color.g, color.b], i * 3);
  }}
  const geometry = new THREE.BufferGeometry();
  geometry.setAttribute('position', new THREE.BufferAttribute(positions, 3));
  geometry.setAttribute('color', new THREE.BufferAttribute(colors, 3));
  geometry.setIndex(data.triangles);
  const material = new THREE.MeshStandardMaterial({{ vertexColors: true, roughness: 0.82, metalness: 0.0, side: THREE.DoubleSide }});
  const mesh = new THREE.Mesh(geometry, material);
  scene.add(mesh);
  scene.add(new THREE.HemisphereLight(0xffffff, 0x444444, 2.1));
  const key = new THREE.DirectionalLight(0xffffff, 2.4);
  key.position.set(3, 4, 5);
  scene.add(key);

  const controls = new OrbitControls(camera, renderer.domElement);
  controls.enableDamping = true;
  controls.dampingFactor = 0.08;

  const applyPose = (name) => {{
    const source = data[name];
    for (let i = 0; i < source.length; i++) {{
      positions[i * 3] = source[i][0];
      positions[i * 3 + 1] = source[i][2];
      positions[i * 3 + 2] = -source[i][1];
    }}
    geometry.attributes.position.needsUpdate = true;
    geometry.computeVertexNormals();
    root.querySelectorAll('[data-pose]').forEach(button => {{
      const active = button.dataset.pose === name;
      button.classList.toggle('btn-primary', active);
      button.setAttribute('aria-pressed', String(active));
    }});
  }};
  const resetView = () => {{
    geometry.computeBoundingBox();
    const center = geometry.boundingBox.getCenter(new THREE.Vector3());
    const size = geometry.boundingBox.getSize(new THREE.Vector3()).length();
    controls.target.copy(center);
    camera.position.set(center.x + size * 0.65, center.y + size * 0.18, center.z + size * 1.15);
    camera.near = Math.max(size / 100, 0.01);
    camera.far = size * 20;
    camera.updateProjectionMatrix();
    controls.update();
  }};
  root.querySelectorAll('[data-pose]').forEach(button => button.addEventListener('click', () => applyPose(button.dataset.pose)));
  root.querySelector('#gaters-wireframe').addEventListener('change', event => {{ material.wireframe = event.target.checked; }});
  root.querySelector('[data-reset]').addEventListener('click', resetView);

  const resize = () => {{
    const width = Math.max(viewport.clientWidth, 320);
    const height = viewport.clientHeight;
    renderer.setSize(width, height, false);
    camera.aspect = width / height;
    camera.updateProjectionMatrix();
  }};
  new ResizeObserver(resize).observe(viewport);
  applyPose('rest');
  resetView();
  resize();
  const animate = () => {{ controls.update(); renderer.render(scene, camera); requestAnimationFrame(animate); }};
  animate();
</script>
'''
    output = Path(output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(fragment, encoding="utf-8")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("readback", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    render(json.loads(args.readback.read_text(encoding="utf-8")), args.output)
    print(f"MODEL_INSPECTOR_PASS output={args.output}")


if __name__ == "__main__":
    main()
