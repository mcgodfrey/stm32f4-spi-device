"""
SPI controller test script for stm32f4-spi-device
https://github.com/mcgodfrey/stm32f4-spi-device

To use: 
- connect raspberry pi SPI pins to stm32f4 discovery board running the firmware at https://github.com/mcgodfrey/stm32f4-spi-device
- RPI GPIO pin 17 is used as CS
- python3 stm32_spi_test.py

Expected output:
-- Reading from device
   device wants to send 14 bytes
   response = "initial string"
-- Writing to device
   sending "this is a write test"
-- Reading from device
   device wants to send 20 bytes
   response = "this is a write test"

"""
import RPi.GPIO as GPIO
import spidev

GPIO.setmode(GPIO.BCM)  # Sets the GPIO pin labelling mode
SPI_SPEED = 10000  # SPI speed in Hz
spi = spidev.SpiDev()


def setup()
    """Setup GPIO port for CS pin, and the SPI bus"""
    # setup CS
    CS_PIN = 17
    GPIO.setup(CS_PIN, GPIO.OUT)
    GPIO.output(CS_PIN, 1)  # set CS initially to high. CS is pulled low to start a transfer

    # setup SPI
    spi.open(0, 0)  # we are manually toggling CS, so just select channel 0 (ignored)
    spi.max_speed_hz = SPI_SPEED
    spi.mode = 0


def read():
    """ perform a read from the device"""
    print('-- Reading from device')
    rw_byte = 0x00
    nbytes = 0x00
    
    command_packet = bytes([rw_byte, nbytes])
    response = _spi_xfer(command_packet)
    nbytes = response[1]
    print(f'   device wants to send {nbytes} bytes')
    
    data_packet = bytes(nbytes)  # initialises a byte array of all zeros, with length = nbytes
    response = _spi_xfer(data_packet).decode('utf-8')
    print(f'   response = "{response}"')


def write(data: str):
    """Write data to the device"""
    print(f'-- Writing to device')
    print(f'   sending "{data}"')
    rw_byte = 0x01
    nbytes = len(data)

    command_packet = bytes([rw_byte, nbytes])
    response = _spi_xfer(command_packet)
    data_packet = data.encode('utf-8')
    _spi_xfer(data_packet)

    
def _spi_xfer(to_write: bytes):
    """
    SPI transfer. 
    Toggles the CS pin, and transfers the data in to_write, while reading from the device.
    To read from the device, set to_write to all zeros
    """
    # assert CS
    GPIO.output(CS_PIN, 0)
    
    # do the actual transfer
    response = bytes(spi.xfer2(to_write))
    
    # release CS
    GPIO.output(CS_PIN, 1)
    
    return response
    

if __name__ == '__main__':
    setup()
    read()
    write('this is a write test')
    read()
    GPIO.cleanup()
