class ImportFunctions:

    def __init__(self):
        self.value = 77
        self.list = [19.0, 17.0, 44.0]

    def return_val(self):
        return self.value

    def return_list(self):
        return self.list

    def change_state(self):
        self.value *= 2.0
        self.list = [x * 2.0 for x in self.list]