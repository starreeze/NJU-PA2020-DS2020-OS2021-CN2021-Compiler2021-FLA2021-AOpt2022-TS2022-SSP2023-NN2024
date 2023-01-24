import copy
import random

import numpy as np
import math
import cv2
from tqdm import trange

image_shape = 160


def read_img(imgpath):
    # img = cv2.imread("cameraman.tif", 2)
    img = cv2.imread(imgpath, 2)
    # img = cv2.pyrDown(img)
    img = cv2.resize(img, (image_shape, image_shape))
    # img = cv2.pyrDown(img)
    return img


def set_seed(seed):
    random.seed(seed)
    np.random.seed(seed)


def generate_random_blur_matrix(shape=64):
    set_seed(1)
    R = np.random.rand(shape, shape) / 1e4 + np.eye(shape)
    R = R / np.max(R.flatten())
    return R


def gen_blur_matrix(shape=64, radius=5):
    R = np.eye(shape + radius * 2, dtype=np.float32) * 1.0
    
    blur_vector = np.linspace(0, radius, radius)
    blur_vector = np.power(0.4, blur_vector).astype(np.float32) * 0.3
    blur_vector = blur_vector.reshape(radius, )
    symmetry_blur_vector = blur_vector[::-1].reshape(radius, )
    
    blur_vector = np.hstack((symmetry_blur_vector, blur_vector[1:])).astype(np.float32)
    print(blur_vector)
    for i in trange(radius, shape, 1):
        for j in range(-(radius - 1), radius, 1):
            if i + j * image_shape - (radius - 1) < 0 and i + j * image_shape + (radius - 1) + 1 >= 0:
                R[i, i + j * image_shape - (radius - 1):] += blur_vector[:-(i + j * image_shape - (radius - 1))]
                R[i, :i + j * image_shape + (radius - 1) + 1] += blur_vector[-(i + j * image_shape - (radius - 1)):]
            else:
                if R[i, i + j * image_shape - (radius - 1):i + j * image_shape + (radius - 1) + 1].shape[0] == blur_vector.shape[0]:
                    R[i, i + j * image_shape - (radius - 1):i + j * image_shape + (radius - 1) + 1] += blur_vector
    
    R = R / np.max(R.flatten())
    R = R[radius:shape + radius, radius:shape + radius]
    return R


if __name__ == '__main__':
    # R = generate_random_blur_matrix(image_shape ** 2)
    # R = generate_blur_matrix(shape=image_shape ** 2, radius=10)
    R = gen_blur_matrix(shape=image_shape ** 2, radius=3)
    
    imgname = 'eason'
    # imgname = 'cameraman'
    
    if imgname == 'cameraman':
        img = read_img('data/cameraman.png')
    elif imgname == 'eason':
        img = read_img('data/{}.jpeg'.format(imgname))
    img = img.flatten()
    img = img.reshape((1, image_shape ** 2))
    img = img / np.max(img.flatten())
    # img = img / 256.0
    img = img.astype(np.float32)
    
    ori_img = copy.deepcopy(img)
    
    img = img @ R
    # img = R @ img.T
    
    img = img.reshape((image_shape, image_shape))
    img = img / np.max(img.flatten())
    img = img * 256.0
    
    ori_img = ori_img.reshape((image_shape, image_shape))
    ori_img = ori_img * 256.0
    cv2.imwrite('data/blurred_{}.png'.format(imgname), img)
    cv2.imwrite('data/ori_{}.png'.format(imgname), ori_img)
    print('Blurring finished.')
