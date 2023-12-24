import sys

if (len(sys.argv) != 4):
    print ("Unexpected parameters. Please run the python script as 'python xxx.py file.txt $(origin content) $(new content)'")
    exit(-1)

with open(sys.argv[1], 'r', encoding='utf-8') as file:
    content = file.read()

newContent = content.replace(sys.argv[2], sys.argv[3])

with open(sys.argv[1], 'w', encoding='utf-8') as file:
    file.write(newContent)