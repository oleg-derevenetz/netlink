An example of data exchange using the Generic Netlink protocol between two userspace applications using `libnl`.

```text
$ sudo ./server
Listening on port 3766496068 ...
```

```text
$ sudo ./client 3766496068 '{"action":"add","argument_1":1,"argument_2":2}'
{
 "result": 3.0
}

$ sudo ./client 3766496068 '{"action":"sub","argument_1":1,"argument_2":2}'
{
 "result": -1.0
}

$ sudo ./client 3766496068 '{"action":"mul","argument_1":1,"argument_2":2}'
{
 "result": 2.0
}

$ sudo ./client 3766496068 '{"action":"div","argument_1":1,"argument_2":2}'
{
 "result": 0.5
}

$ sudo ./client 3766496068 '{"action":"nop","argument_1":1,"argument_2":2}'
{
 "error": "Invalid action"
}

$ sudo ./client 3766496068 '{}'
{
 "error": "Invalid json"
}

$ sudo ./client 3766496068 ''
{
 "error": "Invalid json"
}
```
