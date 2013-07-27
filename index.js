var arDrone = require('ar-drone')
var net = require('net')
var split = require('split')
var through = require('through')

var drone = arDrone.createClient()

var client = net.connect({
  port: 12345
})
.pipe(split())
.pipe(through(function (line) {
  var arr = line.split(',')
  console.log(arr)
  var x = arr[0], y = arr[1], z = arr[2]
  set('x', x)
  set('y', y)
  set('z', z)
}))

drone.disableEmergency()
drone.stop() // Stop command drone was executing before batt died

drone.on('batteryChange', function (num) {
  console.log('battery: ' + num)
})

var params = { x: 0, y: 0, z: 0, rot: 0 }

function set (param, val) {
  if (val > 1)
    params[param] = 1
  else if (val < -1)
    params[param] = -1
  else
    params[param] = val

  if (param === 'x')
    drone.front(params[param])
  else if (param === 'y')
    drone.right(params[param])
  else if (param === 'z')
    drone.up(params[param])
  else if (param === 'rot')
    drone.clockwise(params[param])
  else
    console.error('Invalid param to `set`')
}

function reset () {
  ['x', 'y', 'z', 'rot'].forEach(function (param) {
    set(param, 0)
  })
}

drone.on('navdata', function (data) {
  console.log(data)
})