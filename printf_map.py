import gdb

class MapPrinter(gdb.Command):
    def __init__(self):
        super(MapPrinter, self).__init__("print_map", gdb.COMMAND_USER)
    
    def invoke(self, arg, from_tty):
        map_var = gdb.parse_and_eval(arg)
        map_size = gdb.parse_and_eval(f"{arg}.size()")
        print(f"Map size: {map_size}")

        if map_size == 0:
            print("Map is empty.")
            return
        
        # Get iterators for beginning and end
        begin = gdb.parse_and_eval(f"{arg}.begin()")
        end = gdb.parse_and_eval(f"{arg}.end()")
        
        # Traverse the map using the iterator
        iterator = begin
        while iterator != end:
            # Access the std::pair through the iterator
            node = gdb.parse_and_eval(f"*{iterator}")
            
            # Print out the contents of the std::pair
            try:
                key = node['first']
                value = node['second']
                print(f"Key: {key.string()}, Value: {value.string()}")
            except gdb.error as e:
                print(f"Error accessing elements: {e}")
                print(f"Node: {node}")

            # Move iterator to the next element
            iterator = gdb.parse_and_eval(f"{iterator}++")

MapPrinter()

