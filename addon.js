if (require('os').platform() === 'win32') {
    module.exports = {
        ...require('./build/Release/windows-utils.node'),
    }
} else {
    module.exports = {

    }
}
