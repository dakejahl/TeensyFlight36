import plotly.express as px
iris = px.data.iris()
print(iris)
fig = px.scatter_3d(iris, x='sepal_length', y='sepal_width', z='petal_width',
              color='species')
fig.show()
