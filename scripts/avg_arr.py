import numpy as np

print(
    list((
            np.array([5, 4, 5, 2, 2, 2, 0, 6, 1, 3]) + 
            np.array([4, 6, 8, 5, 0, 6, 6, 6, 3, 2]) + 
            np.array([4, 5, 6, 3, 0, 3, 5, 0, 0, 0]) + 
            np.array([3, 3, 4, 3, 3, 6, 1, 0, 2, 0]) + 
            np.array([1, 3, 2, 4, 3, 6, 3, 0, 0, 0]) + 
            np.array([3, 5, 6, 5, 3, 6, 2, 5, 1, 0]) + 
            np.array([5, 5, 5, 2, 0, 7, 0, 4, 4, 3])
        ) / 7
    )
)