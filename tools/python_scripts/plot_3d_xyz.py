import pandas as pd
import plotly.express as px

df = pd.read_csv('mag_data.csv')
print(df)

# iris = px.data.iris()
# print(iris)
fig = px.scatter_3d(df, x='x', y='y', z='z')

fig.update_traces(marker=dict(size=2,
                              line=dict(width=2,
                                        color='DarkSlateGrey')),
                  selector=dict(mode='markers'))
# fig.show()
fig.write_html('mag_data.html', auto_open=True)
