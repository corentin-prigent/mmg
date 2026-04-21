import matplotlib.pyplot as plt

import numpy as np

import numpy.linalg as nla

import scipy.linalg as sla



from matplotlib.patches import Ellipse





def plot_vector(ax: plt.Axes, start_point: np.ndarray, vector: np.ndarray, **properties) :

    ax.quiver(start_point[0], start_point[1], vector[0], vector[1], **properties)



def plot_metric(ax: plt.Axes, metric1, color1, xlim_ref) :

    

    eigval, eigvec = metric1

    # print("linalg.eig: ", eigval, eigvec)



    eigvec = eigvec.transpose()

    sorted_indices = np.flip(np.argsort(eigval))

    sorted_eigval = eigval[sorted_indices]

    sorted_eigvec = eigvec[sorted_indices]



    h1 = 1./np.sqrt(sorted_eigval[0])

    h2 = 1./np.sqrt(sorted_eigval[1])

    semi_axis1 = sorted_eigvec[0,:]

    semi_axis2 = sorted_eigvec[1,:]

    angle = np.arctan2(semi_axis1[1], semi_axis1[0])



    # print("Eigenvector ", sorted_eigvec[0,:], " associated with 1st eigenvalue ", sorted_eigval[0])

    # print("Eigenvector ", sorted_eigvec[1,:], " associated with 2nd eigenvalue ", sorted_eigval[1])

    # print("Semi-axis ", semi_axis1, " of size ", h1)

    # print("Semi-axis ", semi_axis2, " of size ", h2)

    # print("Rotation of angle ", angle)



    ell1 = Ellipse((0, 0), 2*h1, 2*h2, angle=np.rad2deg(angle), facecolor="none", edgecolor=color1)

    xlim1 = 1.2*max(h1,h2)

    xlim = max(xlim_ref, xlim1)

    ax.add_patch(ell1)



    BASIS_VECTOR_PROPERTIES = {

        'scale_units' : 'xy', 

        'angles' : 'xy',

        'scale' : 1,

        'color' : color1,

        'width' : 0.003,

        'linestyle' : '-.'

    }



    plot_vector(ax, np.array([0,0]), h1*semi_axis1, **BASIS_VECTOR_PROPERTIES)

    plot_vector(ax, np.array([0,0]), h2*semi_axis2, **BASIS_VECTOR_PROPERTIES)



    ax.set(xlim=(-xlim, xlim), ylim=(-xlim, xlim), aspect="equal")



    return xlim



def rotate_metric(metric_1, metric_2):



    eigval, eigvec = metric_1

    eigvec = eigvec.transpose()

    sorted_indices = np.flip(np.argsort(eigval))

    sorted_eigval1 = eigval[sorted_indices]

    sorted_eigvec1 = eigvec[sorted_indices]



    eigval, eigvec = metric_2

    eigvec = eigvec.transpose()

    sorted_indices = np.flip(np.argsort(eigval))

    sorted_eigval2 = eigval[sorted_indices]

    sorted_eigvec2 = eigvec[sorted_indices]



    # ps1 = sorted_eigvec1[0,:].dot(-sorted_eigvec2[0,:])

    # ps2 = sorted_eigvec1[0,:].dot(sorted_eigvec2[0,:])

    # if (ps1 > ps2) :

    #     sorted_eigvec2[0,:] = -sorted_eigvec2[0,:]



    # ps1 = sorted_eigvec1[1,:].dot(-sorted_eigvec2[1,:])

    # ps2 = sorted_eigvec1[1,:].dot(sorted_eigvec2[1,:])

    # if (ps1 > ps2) :

    #     sorted_eigvec2[1,:] = -sorted_eigvec2[1,:]



    sorted_eigvec2[0,:] = -sorted_eigvec2[0,:]

    # met2  = sorted_eigvec2.transpose().dot(np.diag(sorted_eigval2)).dot(sorted_eigvec2)

    met2 = (sorted_eigval2, sorted_eigvec2.transpose())



    return met2





def intersect_metrics(metric_1, metric_2):



    m1_sqrtInv = sla.fractional_matrix_power(metric_1,-0.5)

    m1_bar = m1_sqrtInv.transpose().dot(metric_1).dot(m1_sqrtInv)

    m2_bar = m1_sqrtInv.transpose().dot(metric_2).dot(m1_sqrtInv)



    eigval, eigvec = nla.eig(m2_bar)



    eigval_max = [max(1,eigval[0]), max(1,eigval[1])]

    lbd_max = np.diag(eigval_max)



    mint_bar = eigvec.dot(lbd_max).dot(eigvec.transpose())



    m1_sqrt = sla.fractional_matrix_power(metric_1,0.5)

    metric_int = m1_sqrt.transpose().dot(mint_bar).dot(m1_sqrt)



    # print(m1_bar)

    # m1 = eigvec.dot(m1_bar).dot(eigvec.transpose())

    # print(m1)



    return metric_int





def average_metrics(metric_1: np.ndarray, metric_2: np.ndarray):



    metric_int = 0.5*(metric_1+metric_2)



    return metric_int





def average_metrics_axis(metric_1, metric_2) :

    

    eigval, eigvec = metric_1

    eigvec = eigvec.transpose()

    h1 = 1./np.sqrt(eigval[0])

    h2 = 1./np.sqrt(eigval[1])

    semi_axis11 = eigvec[0,:]

    semi_axis12 = eigvec[1,:]



    eigval, eigvec = metric_2

    eigvec = eigvec.transpose()

    h1 = 1./np.sqrt(eigval[0])

    h2 = 1./np.sqrt(eigval[1])

    semi_axis21 = eigvec[0,:]

    semi_axis22 = eigvec[1,:]



    semi_axis1 = 0.5*(semi_axis11 + semi_axis21)

    semi_axis2 = 0.5*(semi_axis12 + semi_axis22)

    h1 = nla.norm(semi_axis1)

    h2 = nla.norm(semi_axis2)

    eigval = np.array([1./(h1*h1), 1./(h2*h2)])

    eigvec[:,0] = semi_axis1/h1

    eigvec[:,1] = semi_axis2/h2



    return (eigval, eigvec)





metric1 = np.array([[3,0.],[0.,2]])

eigval, eigvec = nla.eig(metric1)

metric1_ev = (eigval, eigvec)



metric2 = np.array([[1.5,1.4],[1.4,2]])

eigval, eigvec = nla.eig(metric2)

metric2_ev = (eigval, eigvec)

eigval, eigvec = rotate_metric(metric1_ev, metric2_ev)

metric2_ev = (eigval, eigvec)



metric_int = intersect_metrics(metric1, metric2)

eigval, eigvec = nla.eig(metric_int)

metric_int_ev = (eigval, eigvec)

eigval, eigvec = rotate_metric(metric1_ev, metric_int_ev)

metric_int_ev = (eigval, eigvec)



metric_avg = average_metrics(metric1, metric2)

eigval, eigvec = nla.eig(metric_avg)

metric_avg_ev = (eigval, eigvec)

eigval, eigvec = rotate_metric(metric1_ev, metric_avg_ev)

metric_avg_ev = (eigval, eigvec)



eigval, eigvec = average_metrics_axis(metric1_ev, metric2_ev)

metric_avg2_ev = (eigval, eigvec)





fig, ax = plt.subplots()

xl = plot_metric(ax, metric1_ev, 'b', 0)

xl = plot_metric(ax, metric2_ev, 'r', xl)

#xl = plot_metric(ax, metric_avg_ev, 'g', xl)

xl = plot_metric(ax, metric_int_ev, 'm', xl)

#xl = plot_metric(ax, metric_avg2_ev, 'c', xl)

plt.show()









# eigval, eigvec = nla.eig(metric2)

# eigvec = eigvec.transpose()

# sorted_indices = np.flip(np.argsort(eigval))

# sorted_eigval = eigval[sorted_indices]

# sorted_eigvec = eigvec[sorted_indices]

# met = sorted_eigvec.dot(np.diag(sorted_eigval)).dot(sorted_eigvec.transpose())

# print(met)

# met = sorted_eigvec.transpose().dot(np.diag(sorted_eigval)).dot(sorted_eigvec)

# print(met)