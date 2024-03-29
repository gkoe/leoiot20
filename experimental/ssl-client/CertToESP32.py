#MIT License(MIT)

#     CertToHex.py Version 1.0.0        #

#     Copyright(c) 2018 Mike Simpson      #

#Permission is hereby granted, free of charge, to any person obtaining a copy
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:

#The above copyright notice and this permission notice shall be included in all
#copies or substantial portions of the Software.

#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#SOFTWARE.

import binascii
filename = 'LetsEncryptRoot.cer'
with open(filename, 'rb') as f:
    content = f.read()

print('// '+filename)
print('const char* test_root_ca = \ ')
outString = '"'
caCertLen = 0


x = len(content)

for i in range(0, x-1):
    first = (chr(content[i]))
 #   print(first,content[i])
    if content[i]==13:
        outString = outString + '\\'
    outString = outString+first
    if content[i]==10:
        outString = outString + ''
    
outString = outString[:-2] #remove last comma and space

print(outString[:-1]+';')
