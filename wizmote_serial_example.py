import wizmote_serial



def all_all(data):
  print("All remotes all buttons, JSON")
  print(data)
  print()



def all_1(data):
  print("All remotes, 1 button, str")
  print(data["button"])
  print()



def one_all(data):
  print("One remote, all buttons, JSON")
  print(data)
  print()



def all_all2(data):
  print("All remotes all buttons, variables")
  remote_id  = data["remote_id"]
  message_id = data["message_id"]
  button_id  = data["button_id"]
  button     = data["button"]
  print("%s says %s" % (remote_id, button))
  print()



wm = wizmote_serial.wizmote()

wm.register_callback(all_all)
wm.register_callback(all_1, button="1")
#wm.register_callback(one_all, remote_id="444f8exxxxxx")
wm.register_callback(all_all2)

wm.run()
