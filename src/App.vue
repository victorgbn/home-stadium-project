<template>
  <div id="app">
    {{data}}
      <div class="">

      </div>
  </div>
</template>

<script>
import axios from 'axios'
export default {
  name: 'App',
  components: {

  },
  data () {
    return {
      data: null, 
      team1: null, 
      team
    }
  },
  mounted () {
    axios
      .get('api.json')
      .then(response => (this.data = response))
  }
}
</script>

<style>
#app {
  font-family: Avenir, Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: #2c3e50;
  margin-top: 60px;
}
</style>
