var arDrone = require('ar-drone')
var client = arDrone.createClient()

client.disableEmergency()
client.stop() // Stop command drone was executing before batt died

client.on('batteryChange', function (num) {
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
    client.front(params[param])
  else if (param === 'y')
    client.right(params[param])
  else if (param === 'z')
    client.up(params[param])
  else if (param === 'rot')
    client.clockwise(params[param])
  else
    console.error('Invalid param to `set`')
}

function reset () {
  ['x', 'y', 'z', 'rot'].forEach(function (param) {
    set(param, 0)
  })
}

