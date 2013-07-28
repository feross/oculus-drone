var arDrone = require('ar-drone')
var fs = require('fs')
var net = require('net')
var optimist = require('optimist')
var PaVEParser = require('./node_modules/ar-drone/lib/video/PaVEParser')
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
  .argv

var drone = arDrone.createClient()
var state = { x: 0, y: 0, z: 0, rot: 0 }
var inAir = false
var gestureEnabled = true

drone.disableEmergency()
drone.stop() // Stop command drone was executing before batt died

drone.on('batteryChange', function (num) {
  console.log('battery: ' + num)
})

setInterval(function () {
  console.log(state)
}, 1000)

function set (params) {
  for (var param in params) {
    var val = params[param]

    // Update parameter
    if (val > 1) {
      state[param] = 1
    } else if (val < -1) {
      state[param] = -1
    } else {
      state[param] = val
    }

    // Send movement command to drone
    if (param === 'x') {
      drone.front(val)
    } else if (param === 'y') {
      drone.right(val)
    } else if (param === 'z') {
      drone.up(val)
    } else if (param === 'rot') {
      drone.clockwise(val)
    } else {
      console.error('Invalid param to `set`')
    }
  }
}

function reset () {
  set({x: 0, y: 0, z: 0, rot: 0})
  drone.stop()
}

function takeoff () {
  drone.disableEmergency()
  drone.stop()
  drone.takeoff()
  inAir = true
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

  if (key === 'k') {
    land()
    setTimeout(function () {
      process.exit(0)
    }, 200)

  if (key === 'w') {
    set({ x: speed })
  } else if (key === 's') {
    set({ x: -speed })
  } else if (key === 'd') {
    set({ y: speed })
  } else if (key === 'a') {
    set({ y: -speed })
  } else if (UP) {
    set({ z: speed })
  } else if (DOWN) {
    set({ z: -speed })
  } else if (LEFT) {
    set({ rot: -speed }) // COUNTERCLOCKWISE
  } else if (RIGHT) {
    set({ rot: speed }) // CLOCKWISE
  } else if (key === 'e') {
    drone.stop()
  } else if (key === 't') {
    takeoff()
  } else if (key === 'l') {
    land()

  } else if (keyBuf[0] === 32) {
    flipAhead()
  }
})

if (argv.oculus) {
  // Connect to oculus server
  var oculusClient = net.connect({ port: 12345 })
  oculusClient.on('error', function (err) {
    console.error('Error connecting to oculus server.')
  })

  oculusClient
    .pipe(split())
    .pipe(through(function (line) {
      var arr = line.split(',')
      var x = Number(-arr[0]), rot = Number(-arr[1]), y = Number(-arr[2])
      set({ x: x, y: y, rot: rot })

      // Gestures:
      //   DOWN to takeoff/land
      //   UP to flip
      if (gestureEnabled) {
        if (x < -1) {
          flipBehind()
          disableGestureTimeout()
        } else if (x > 1) {
          if (inAir) {
            land()
          } else {
            takeoff()
          }
          disableGestureTimeout()
        }
      }
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



// drone.on('navdata', function (data) {
//   console.log(data)
// })

