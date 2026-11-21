a = 1
def f():
    global a
    a += 1
    return 2
a += f()
print(a)
