import heapq
from collections import defaultdict

class Node(object):
    def __init__(self, data=None, weight=None) -> None:
        self.data   = data
        self.weight = weight
        self.left   = None
        self.right  = None

    def __lt__(self, other):
        return self.weight < other.weight

    def __repr__(self):
        return f"Node(data={self.data}, weight={self.weight})"


def huffman_tree(freq):
    heap = [Node(data=char, weight=weight) for char, weight in freq.items()]
    heapq.heapify(heap)

    while len(heap) > 1:
        left  = heapq.heappop(heap)
        right = heapq.heappop(heap)

        merged       = Node(data='$', weight=left.weight + right.weight)
        merged.left  = left
        merged.right = right

        heapq.heappush(heap, merged)

    return heap[0]

def is_leaf(node):
    return node.left is None and node.right is None

def store_codes(root, current_code, codes):
    if root is None:
        return
    
    if is_leaf(root):
        codes[root.data] = current_code

    store_codes(root.left, current_code + '0', codes)
    store_codes(root.right, current_code + '1', codes)

def compress(input_string):
    counts = defaultdict(int)
    for c in input_string:
        counts[c] += 1

    root  = huffman_tree(counts)
    codes = {}
    store_codes(root, '', codes)

    compressed_string = ''.join(codes[c] for c in input_string)
    return compressed_string

def main():
    path = input('Enter file path: ')

    if path == '':
        print('File path is empty')
        return -1
    
    try:
        with open(path, 'r') as file:
            content = file.read().strip()
    except FileNotFoundError:
        print(f'File {path} not found')
        return -1

    compressed_output = compress(content)
    out_path          = 'compressed_file.txt'

    with open(out_path, 'w') as file:
        file.write(compressed_output)
    
    print(f'File compressed successfully to {out_path}')
    return 0

if __name__ == '__main__':
    main()