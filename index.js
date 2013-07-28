var arDrone = require('ar-drone')
var fs = require('fs')
var net = require('net')
var optimist = require('optimist')
var PaVEParser = require('./node_modules/ar-drone/lib/video/PaVEParser')
var PidController = require('node-pid-controller')
var split = require('split')
var through = require('through')

// Don't crash node on exceptions
process.on('uncaughtException', function (err) {
  console.error(err.stack)
  try {
    land()
  } catch (e) {}
  setTimeout(function () {
    process.exit(0)
  }, 200)
})

var argv = optimist
  .usage('Usage: $0 [--oculus]')
  .alias('oculus', 'o')
  .alias('demo', 'd')
  .argv

var drone = arDrone.createClient()
var speed = {
  x: 0, // forward/back
  y: 0, // left/right
  z: 0, // rotateLeft/rotateRight
  e: 0  // up/down
}
var pid = null
var zero = {
  z: 0
}

function setupPid () {
  pid = new PidController(0.01, 0, 0.0001)
}

// Fixes the 180 to -180 problem
var last = {
  input: 0,
  output: 0
}
function correctZ (zTarget, inOrOut) {
  while(zTarget - last[inOrOut] > 180) {
    zTarget -= 360
  }
  while(last[inOrOut] - zTarget > 180) {
    zTarget += 360
  }
  last[inOrOut] = zTarget
  return zTarget
}

var inAir = false
var gestureEnabled = true

drone.disableEmergency()
drone.stop() // Stop command drone was executing before batt died
drone.config('control:control_yaw', '6.1')
drone.config('control:euler_angle_max', '0.37')

drone.on('batteryChange', function (num) {
  console.log('battery: ' + num)
})

setInterval(function () {
  if (pid) {
    console.log({
      zTarget: pid.target.toFixed(2),
      inAir: inAir
    })
  }
}, 500)

function set (params) {
  for (var param in params) {
    var val = params[param]

    // Update parameter
    if (val > 1) {
      speed[param] = 1
    } else if (val < -1) {
      speed[param] = -1
    } else {
      speed[param] = val
    }

    // Send movement command to drone
    if (param === 'x') {
      drone.front(val)
    } else if (param === 'y') {
      drone.right(val)
    } else if (param === 'e') {
      drone.up(val)
    } else if (param === 'z') {
      drone.clockwise(val)
    } else {
      console.error('Invalid param to `set`')
    }
  }
}

function reset () {
  set({x: 0, y: 0, z: 0, e: 0})
  drone.stop()
}

function takeoff () {
  drone.disableEmergency()
  drone.stop()
  drone.takeoff()
  inAir = true
  setupPid()
}

function land () {
  inAir = false
  drone.land()
}

function flipAhead () {
  drone.animate('flipAhead', 500)
}
function flipBehind () {
  drone.animate('flipBehind', 500)
}
function flipLeft () {
  drone.animate('flipLeft', 500)
}
function flipRight () {
  drone.animate('flipRight', 500)
}

function disableGestureTimeout () {
  gestureEnabled = false
  setTimeout(function () {
    gestureEnabled = true
  }, 2000)
}

// Keyboard control
process.stdin.setRawMode(true)
process.stdin.on('data', function(chunk) {
  var key = chunk.toString()
  var keyBuf = chunk.toJSON()
  var speed = 0.2

  console.log(key)
  console.log(keyBuf)

  if (Array.isArray(keyBuf)) {
    var UP = (keyBuf[0] === 27 && keyBuf[1] === 91 && keyBuf[2] === 65)
    var DOWN = (keyBuf[0] === 27 && keyBuf[1] === 91 && keyBuf[2] === 66)
    var RIGHT = (keyBuf[0] === 27 && keyBuf[1] === 91 && keyBuf[2] === 67)
    var LEFT = (keyBuf[0] === 27 && keyBuf[1] === 91 && keyBuf[2] === 68)
  }

  if (key === 'w') {
    set({ x: speed })
  } else if (key === 's') {
    set({ x: -speed })
  } else if (key === 'd') {
    set({ y: speed })
  } else if (key === 'a') {
    set({ y: -speed })
  } else if (UP) {
    set({ e: speed })
  } else if (DOWN) {
    set({ e: -speed })
  } else if (LEFT) {
    if (argv.oculus) {
      zero.z += 5
    } else {
      set({ z: -speed }) // COUNTERCLOCKWISE
    }
  } else if (RIGHT) {
    if (argv.oculus) {
      zero.z -= 5
    } else {
      set({ z: speed }) // CLOCKWISE
    }
  } else if (key === 'e') {
    drone.stop()
  } else if (key === 'k') {
    land()
    setTimeout(function () {
      process.exit(0)
    }, 200)
  } else if (key === 't') {
    takeoff()
  } else if (key === 'l') {
    land()

  } else if (keyBuf[0] === 32) {
    flipAhead()
  }
})

// Oculus control
if (argv.oculus) {

  // Connect to oculus server
  var oculusClient = net.connect({ port: 12345 })
  oculusClient.on('error', function (err) {
    console.error('Error connecting to oculus server.')
  })

  // Process oculus data
  oculusClient
    .pipe(split())
    .pipe(through(function (line) {
      var arr = line.split(',')
      var xTarget = Number(-arr[0] * (180 / Math.PI))
      var yTarget = Number(-arr[2] * (180 / Math.PI))
      var zTarget = Number(-arr[1] * (180 / Math.PI))

      if (inAir) {
        set({ x: (xTarget / 90),
              y: (yTarget / 90) })
        pid.setTarget(correctZ(zTarget, 'output'))
      }

      // Gestures:
      //   DOWN to takeoff/land
      //   UP to flip
      // if (gestureEnabled) {
      //   if (xTarget < -1) {
      //     flipBehind()
      //     disableGestureTimeout()
      //   } else if (xTarget > 1) {
      //     if (inAir) {
      //       land()
      //     } else {
      //       takeoff()
      //     }
      //     disableGestureTimeout()
      //   }
      // }
    }))
}

// Video server
net.createServer(function (c) {
  console.log('server connected')

  c.on('end', function() {
    console.log('server disconnected')
  })

  drone.getVideoStream()
    .pipe(new PaVEParser())
    .pipe(through(function (data) {
      this.queue(data.payload)
      console.log('video bits sent')
    }))
    .pipe(c)

}).listen(6969)

drone.on('navdata', function (data) {
  var zSensor = Number(data.demo.rotation.z)

  zSensor = correctZ(zSensor, 'input')

  // Drone starting position should be 0,0,0.
  if (inAir) {
    zSensor += zero.z

    var zCorrect = pid.update(zSensor)

    set({ z: zCorrect })

    console.log('sensor: { z: ' + zSensor.toFixed(2) + ' } correct: { z: ' + zCorrect.toFixed(2) + ' }')
  }
})

if (argv.demo) {
  takeoff()
  drone
    .after(5000, function () {
      set({ x: (1 / 30) })
    })
    .after(3000, function () {
      set({ x: -(1 / 30) })
    })
    .after(3000, function () {
      set({ x: (1 / 30) })
    })
    .after(5000, function () {
      set({ x: 0 })
    })
    .after(2000, function () {
      land()
    })
}
