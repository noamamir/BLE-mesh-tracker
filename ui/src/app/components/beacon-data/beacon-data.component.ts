import {Component, Input, OnChanges, OnDestroy, OnInit, SimpleChanges} from '@angular/core';
import {Beacon} from "../../models/beacon";
import {Observable} from "rxjs";
import {TimeElapsedService} from "../../services/timeElapsed/time-elapsed.service";

@Component({
  selector: 'app-beacon-data',
  templateUrl: './beacon-data.component.html',
  styleUrls: ['./beacon-data.component.css']
})
export class BeaconDataComponent implements OnInit{
  @Input()
  beaconData: Beacon
  time$: Observable<number>;

  constructor(private timeElapsedService: TimeElapsedService) {}

  ngOnInit(): void {
    this.time$ = this.timeElapsedService.time()
  }
}
