import argparse
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import mpl_toolkits.mplot3d.axes3d as p3
import numpy as np

def get_results():
    """
    Reads from the piped input, which must be a results file generated from 
    ./nbody with the flags -t 0 and -o 2.

    Returns a list of 2D numpy array that contains data for the position
    and radius of every body every second and a list of the radius of every
    body.
    """
    num_bodies, seconds = [int(i) for i in input().split()]
    results = []
    radii = []

    for i in range(seconds):
        result_this_second = np.zeros((num_bodies, 3))

        for j in range(num_bodies):
            x, y, z, radius = [float(k) for k in input().split()]
            result_this_second[j][0] = x
            result_this_second[j][1] = y
            result_this_second[j][2] = z

            if len(radii) < num_bodies:
                radii.append(radius)

        results.append(result_this_second)

    return results, radii

def update(iteration, results, plot):
    """
    Updates the scatter plot given the current iteration, results,
    and the current plot.
    """
    for i in range(results[0].shape[0]):
        plot[i]._offsets3d = (results[iteration][i,0:1], results[iteration][i,1:2], 
            results[iteration][i,2:])
    return plot

def animate(results, radii, save=False):
    """
    Generates the visualization given the results and radii. Saves the animation
    as a .gif if `save` is True.
    """
    fig = plt.figure()
    ax = p3.Axes3D(fig)
    plt.style.use('dark_background')

    plot = [ax.scatter(results[0][i,0:1], results[0][i,1:2], results[0][i,2:],
        s=radii[i]) for i in range(results[0].shape[0])]
    num_iterations = len(results)

    # Axes
    ax.set_xlim3d([-5000, 5000])
    ax.set_ylim3d([-5000, 5000])
    ax.set_zlim3d([-5000, 5000])
    ax.view_init(25, 10)

    ani = animation.FuncAnimation(fig, update, num_iterations, fargs=(results,
        plot), interval=50, blit=False, repeat=True)

    if save:
        writergif = animation.PillowWriter(fps=60)
        ani.save('figures/random_r100_s600_results.gif', writer=writergif)
    else:
        plt.show()

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-s", dest = "save", help="Saves the animation to a file"
        + "; displays otherwise.", action='store_true')
    parser.set_defaults(feature=False)

    results, radii = get_results()
    animate(results, radii, parser.parse_args().save)
