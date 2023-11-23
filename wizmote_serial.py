user@unknown:/tmp$ cat a.py 
import serial
import json





class wizmote:
  def __init__(self, dev="/dev/ttyUSB0", baud=115200):
    self.dev           = dev
    self.baud          = baud
    self.ser           = None
    self.stopped       = False
    self.callbacks     = []
    self.button_lookup = {"on":1, "off":2, "sleep":3, "1":16, "2":17, "3":18, "4":19, "-":8, "+":9}

    self.connect()



  def connect(self):
    self.ser = serial.Serial(self.dev, self.baud)



  def read(self):
    try:
      data = self.ser.read_until(b"\n")
      data = data.decode().strip()
      return json.loads(data)
    except Exception as e:
      #print("Error decoding data:", e)
      print(data)
      return False



  def register_callback(self, func, remote_id=None, button_id=None, button=None):
    if not callable(func):
      print("Callback registration failed: callback function is not callable")
      return False

    if button_id is not None and button is not None and button_id not in self.button_lookup.values() and button not in self.button_lookup.keys(): # woof
      print("Callback registration failed: button / button_id is not valid")
      return False

    if button in self.button_lookup:
      button_id = self.button_lookup[button]

    self.callbacks.append({"func":func, "remote_id":remote_id, "button_id":button_id, "button":button})
    return True


  def execute_callbacks(self, data):
    data["button"] = [k for k, v in self.button_lookup.items() if v == data["button_id"]][0]

    for callback in self.callbacks:
      call = True
      if callback["remote_id"] is not None and callback["remote_id"] != data["remote_id"]:
        call = False
      if callback["button_id"] is not None and callback["button_id"] != data["button_id"]:
        call = False
      if call:
        callback["func"](data)



  def run(self):
    while not self.stopped:
      data = self.read()
      if not data: continue
      self.execute_callbacks(data)










if __name__ == "__main__":

  def my_function(data):
    print(data)
    #print(data["remote_id"])
    #print(data["message_id"])
    #print(data["button_id"])
    #print(data["button"])
    #print()

  wm = wizmote()
  wm.register_callback(my_function)
  #wm.register_callback(my_function, remote_id="444f8exxxxxx")
  #wm.register_callback(my_function, button="1")
  #wm.register_callback(my_function, remote_id="444f8exxxxxx", button="1")

  wm.run()
