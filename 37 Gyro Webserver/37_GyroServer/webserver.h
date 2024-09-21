const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Tech Talkies Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">

  <style>
  	html {
  font-family: New Times Roman;
  display: inline-block;
  text-align: center;
}
p {
  font-size: 1.2rem;
}
body {
  margin: 0;
}
.topnav {
  overflow: hidden;
  background-color: #0a0808;
  color: #ffffff;
  font-size: 1rem;
}
.content {
  padding: 20px;
}
.card {
  background-color: white;
  outline: 1px solid;
}
.card-title {
  color:#0a0808;
  font-weight: bold;
}
.cards {
  max-width: 800px;
  margin: 0 auto;
  display: grid; grid-gap: 2rem;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
}
.reading {
  font-size: 1.2rem;
}
.cube-content{
  width: 100%;
  background-color: white;
  height: 500px; margin: auto;
  padding-top:2%;
}
#reset{
  border: none;
  color: #ffffff;
  background-color: #0000FF;
  padding: 10px;
  text-align: center;
  display: inline-block;
  font-size: 14px; width: 150px;
  border-radius: 4px;
}
#resetX, #resetY, #resetZ{
  border: none;
  color: #ffffff;
  background-color: #b00404;
  padding-top: 10px;
  padding-bottom: 10px;
  text-align: center;
  display: inline-block;
  font-size: 14px;
  width: 20px;
  border-radius: 4px;
}
  </style>
</head>
<body>
  <div class="topnav">
    <h1>Tech Talkies Gyro Controller</h1>
  </div>
  <div class="content">
    <div class="cards">
      <div class="card">
        <p><span class="reading">X: <span id="rotationX"></span> rad</span>
        <span class="reading">Y: <span id="rotationY"></span> rad</span>
        <span class="reading">Z: <span id="rotationZ"></span> rad</span></p>
		
		<label for="myfile">Select a file:</label>
		<input type="file" id="myfile" name="myfile" accept=".glb"><br><br>
      </div>
    </div>
	<div class="cube-content">
      <div id="3Dcube"></div>
    </div>

 </div>

  <script type="importmap">
          {
            "imports": {
              "three": "https://unpkg.com/three@0.146.0/build/three.module.js"
            }
          }
        </script>
		
 <script type="module">		
		
  const uploadInput = document.getElementById("myfile");
  uploadInput.addEventListener("change", handleFiles, false);
  
 import * as THREE from 'three';
 import { GLTFLoader } from 'https://unpkg.com/three@0.146.0/examples/jsm/loaders/GLTFLoader.js';
 import { OrbitControls } from 'https://unpkg.com/three@0.146.0/examples/jsm/controls/OrbitControls.js';

 let renderer, model, controls;
 let scene, camera, rendered, cube;
 
 function handleFiles() {
  const fileList = this.files; /* now you can work with the file list */
  const selectedFile = document.getElementById("myfile").files[0];
  console.log("File selected");
  const objURL = URL.createObjectURL(selectedFile);
  console.log(objURL);
  
  const loader = new GLTFLoader();
  scene.remove(model);
		//model.removeFromParent();
	 model = new THREE.Object3D();
	 loader.load(objURL, function ( gltf ) {
		model = gltf.scene;
		//model.scale.set(0.01,0.01,0.01);
		scene.add(model);
		render();
	 });
}

function parentWidth(elem) {
  return elem.parentElement.clientWidth;
}

function parentHeight(elem) {
  return elem.parentElement.clientHeight;
}

function initialize_3D(){
	//const container = document.createElement( 'div' );
	//document.body.appendChild( container );
	
	camera = new THREE.PerspectiveCamera(75, parentWidth(document.getElementById("3Dcube")) / parentHeight(document.getElementById("3Dcube")), 0.1, 1000);
    camera.position.set( 0.2, 0.2, 0.2 );

	scene = new THREE.Scene();
    scene.add( new THREE.AmbientLight( 0xffffff, 0.75 ) );

	const dirLight = new THREE.DirectionalLight( 0xffffff, 1 );
	dirLight.position.set( 5, 10, 7.5 );
	dirLight.castShadow = true;
	dirLight.shadow.camera.right = 2;
	dirLight.shadow.camera.left = - 2;
	dirLight.shadow.camera.top	= 2;
	dirLight.shadow.camera.bottom = - 2;
	dirLight.shadow.mapSize.width = 1024;
	dirLight.shadow.mapSize.height = 1024;
	scene.add( dirLight );

    //model
     const loader = new GLTFLoader();
	 model = new THREE.Object3D();
	 loader.load('https://raw.githubusercontent.com/TechTalkies/YouTube/main/37%20Gyro%20Webserver/Jet.glb', function ( gltf ) {
		model = gltf.scene;
        model.name = 'model';
		//model.scale.set(0.01,0.01,0.01);
		scene.add(model);
		render();
	 } );
	 
	renderer = new THREE.WebGLRenderer( { antialias: true } );
            
	renderer.setPixelRatio( window.devicePixelRatio );
	renderer.setSize(parentWidth(document.getElementById("3Dcube")), parentHeight(document.getElementById("3Dcube")));
	renderer.toneMapping = THREE.ACESFilmicToneMapping;
	renderer.toneMappingExposure = 1;
	renderer.outputEncoding = THREE.sRGBEncoding;
	document.getElementById('3Dcube').appendChild( renderer.domElement );

	controls = new OrbitControls( camera, renderer.domElement );
	controls.addEventListener( 'change', render );
    controls.minDistance = 0.001;
	controls.target.set( 0.03, 0.01, -0.01 );
	controls.update();
	window.addEventListener( 'resize', onWindowResize );
}

function onWindowResize() {
	camera.aspect = window.innerWidth / window.innerHeight;
	camera.updateProjectionMatrix();
	renderer.setSize( window.innerWidth, window.innerHeight );
	render();
}

function render() {
	renderer.render( scene, camera );
}

window.addEventListener('resize', onWindowResize, false);

initialize_3D();

if (!!window.EventSource) {
  var source = new EventSource('/events');

  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);

  source.addEventListener('gyro_readings', function(e) {
    var obj = JSON.parse(e.data);
    document.getElementById("rotationX").innerHTML = obj.rotationX;
    document.getElementById("rotationY").innerHTML = obj.rotationY;
    document.getElementById("rotationZ").innerHTML = obj.rotationZ;

    model.rotation.x = obj.rotationY;
    model.rotation.z = obj.rotationX;
    model.rotation.y = obj.rotationZ;
    renderer.render(scene, camera);
  }, false);

}
 </script>
</body>
</html>
)rawliteral";